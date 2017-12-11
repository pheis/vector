#include <array>
#include <cstddef>
//#include <iostream>
#include <memory>

using std::array;
using std::make_pair;
using std::make_shared;
using std::pair;
using std::shared_ptr;

template <class T> class vector {
private:
  static const uint32_t B = 5;
  static const uint32_t M = 1 << B;
  static const uint32_t NIL = ~0;

  static uint32_t get_min_level_count(const uint32_t pop) {
    auto p = pop;
    uint32_t lvl = 0;
    do {
      p = p >> B;
      lvl++;
    } while (p >= 32);
    return lvl;
  }

  template <typename U>
  static array<U, M> copy_except_one(const array<U, M> &old,
                                     const uint32_t &idx, const U some) {
    array<U, M> new_array = array<U, M>();
    for (uint32_t i = 0; i < M; i++) {
      if (i == idx) {
        new_array[i] = some;
      } else {
        new_array[i] = old[i];
      }
    }

    return new_array;
  }

  template <typename U> struct Node {};

  template <typename U> struct Leaf : public Node<U> {
    explicit Leaf<U>() { children = array<U, M>(); }
    explicit Leaf<U>(array<U, M> &children) : children(children) {}
    explicit Leaf<U>(const U &some) {
      children = array<U, M>();
      children[0] = some;
    }
    Leaf<U>(Leaf<U> *old_leaf, const uint32_t l_idx, const T &some) {
      children = copy_except_one(old_leaf->children, l_idx, some);
    }

    array<U, M> children;
  };

  template <typename U> struct Leaf_ptr {
    Leaf_ptr() {
      ptr = nullptr;
      num = NIL;
      read_only = true;
    }
    Leaf<U> *ptr;
    uint32_t num;
    bool read_only;
  };

  template <typename U> struct Branch : public Node<U> {
    Branch<U>()
        : children(array<shared_ptr<Node<U>>, M>()),
          read_only(array<bool, M>()) {}

    Branch<U>(array<shared_ptr<Node<U>>, M> &children,
              array<bool, M> &read_only)
        : children(children), read_only(read_only) {}

    Branch<U>(Branch<U> *old_branch, uint32_t l_idx,
              shared_ptr<Node<T>> &some) {
      children = copy_except_one(old_branch->children, l_idx, some);
      bool foo = false;
      read_only = copy_except_one(old_branch->read_only, l_idx, foo);
    }

    array<shared_ptr<Node<U>>, M> children;
    array<bool, M> read_only;
  };

  static uint32_t local_idx(const uint32_t idx, const uint32_t lvl) {
    return (idx >> (B * lvl)) & (M - 1);
  }

  void mutant_update(Node<T> *ptr, const uint32_t idx, const uint32_t lvl,
                     const T &some) {
    auto l_idx = local_idx(idx, lvl);
    if (lvl == 0) {
      auto leaf = static_cast<Leaf<T> *>(ptr);
      leaf->children[l_idx] = some;
      last_accessed_leaf.num = leaf_number(idx);
      last_accessed_leaf.read_only = false;
      last_accessed_leaf.ptr = leaf;
    } else {
      auto branch = static_cast<Branch<T> *>(ptr);
      if (branch->read_only[l_idx]) {
        // std::cout << "immu upd" << std::endl;
        branch->children[l_idx] =
            immutable_update(branch->children[l_idx].get(), idx, lvl - 1, some);
        branch->read_only[l_idx] = false;
      } else {
        mutant_update(branch->children[l_idx].get(), idx, lvl - 1, some);
      }
    }
  }

  // is the loc idx thing correct here at all?

  shared_ptr<Node<T>> immutable_update(Node<T> *ptr, const uint32_t idx,
                                       const uint32_t lvl, const T &some) {
    auto l_idx = local_idx(idx, lvl);
    if (lvl == 0) {
      Leaf<T> *leaf_ptr = static_cast<Leaf<T> *>(ptr);
      auto new_leaf = make_shared<Leaf<T>>(leaf_ptr, l_idx, some);
      last_accessed_leaf.num = leaf_number(idx);
      last_accessed_leaf.ptr = new_leaf.get();
      last_accessed_leaf.read_only = false;
      return new_leaf;
    } else {
      Branch<T> *branch = static_cast<Branch<T> *>(ptr);

      // spread the immutability lazily downwards
      for (int i = 0; i < M; i++) {
        branch->read_only[i] = true;
      }
      // this path is mutable.
      branch->read_only[l_idx] = false;

      auto node_ptr =
          immutable_update(branch->children[l_idx].get(), idx, lvl - 1, some);
      return make_shared<Branch<T>>(branch, l_idx, node_ptr);
    }
  }

  static Branch<T> grow(Branch<T> &old_root) {
    auto new_root = Branch<T>();
    new_root.children[0] =
        make_shared<Branch<T>>(old_root.children, old_root.read_only);
    return new_root;
  }

  void populate(Node<T> *ptr, const uint32_t idx, const uint32_t lvl,
                const T &some) {
    auto l_idx = local_idx(idx, lvl);
    auto *branch = static_cast<Branch<T> *>(ptr);
    if (branch->children[l_idx] == nullptr) {
      // std::cout << "child  is null" << std::endl;
      // std::cout << "lvl: " << lvl << std::endl;
      if (lvl == 1) {
        //  std::cout << "creating  new leaf" << std::endl;
        branch->children[l_idx] = make_shared<Leaf<T>>(some);
        last_accessed_leaf.ptr =
            static_cast<Leaf<T> *>(branch->children[l_idx].get());
        last_accessed_leaf.num = leaf_number(idx);
        last_accessed_leaf.read_only = false;
      } else {
        branch->children[l_idx] = make_shared<Branch<T>>();
        populate(branch->children[l_idx].get(), idx, lvl - 1, some);
      }
    } else {
      populate(branch->children[l_idx].get(), idx, lvl - 1, some);
    }
  }

  static void construct_children(Node<T> *ptr, const uint32_t last_idx,
                                 const uint32_t lvl) {
    Branch<T> *branch = static_cast<Branch<T> *>(ptr);
    auto local_last_idx = local_idx(last_idx, lvl);

    if (lvl == 1) {
      for (uint32_t i = 0; i <= local_last_idx; i++) {
        branch->children[i] = make_shared<Leaf<T>>();
      }
    } else {
      for (uint32_t i = 0; i < local_last_idx; i++) {
        branch->children[i] = make_shared<Branch<T>>();
        construct_children_helper_full_fill(branch->children[i].get(), lvl - 1);
      }
      branch->children[local_last_idx] = make_shared<Branch<T>>();
      construct_children_helper_partial_fill(
          branch->children[local_last_idx].get(), last_idx, lvl - 1);
    }
  }

  static void construct_children_helper_full_fill(Node<T> *ptr,
                                                  const uint32_t lvl) {
    // std::cout << "Inside function helper_full_fill, lvl: " << lvl <<
    // std::endl;
    Branch<T> *branch = static_cast<Branch<T> *>(ptr);

    if (lvl == 1) {
      // std::cout << "THE M is " << M << std::endl;
      for (uint32_t i = 0; i < M; i++) {
        // std::cout << "Inside function helper_full_fill, creating leaf: " << i
        //          << std::endl;
        branch->children[i] = make_shared<Leaf<T>>();
      }
    } else {
      for (uint32_t i = 0; i < M; i++) {
        branch->children[i] = make_shared<Branch<T>>();
        construct_children_helper_full_fill(branch->children[i].get(), lvl - 1);
      }
    }
  }

  static void construct_children_helper_partial_fill(Node<T> *ptr,
                                                     const uint32_t last_idx,
                                                     const uint32_t lvl) {

    // std::cout << "Inside function helper_partial_fill" << std::endl;
    auto local_last_idx = local_idx(last_idx, lvl);
    Branch<T> *branch = static_cast<Branch<T> *>(ptr);

    if (lvl == 1) {
      // std::cout << "Inside function helper_partial_fill, creating leaves"
      //           << std::endl;
      for (uint32_t i = 0; i <= local_last_idx; i++) {
        branch->children[i] = make_shared<Leaf<T>>();
      }
    } else {
      for (uint32_t i = 0; i < local_last_idx; i++) {
        branch->children[i] = make_shared<Branch<T>>();
        construct_children_helper_full_fill(branch->children[i].get(), lvl - 1);
      }
      branch->children[local_last_idx] = make_shared<Branch<T>>();
      construct_children_helper_partial_fill(
          branch->children[local_last_idx].get(), last_idx, lvl - 1);
    }
  }

  static uint32_t leaf_number(const uint32_t idx) { return (idx >> B); }

  T get_some(Branch<T> *ptr, uint32_t idx, uint32_t lvl, bool ro) {

    auto l_idx = local_idx(idx, lvl);
    bool read_only = ro | ptr->read_only[l_idx];

    if (lvl == 1) {
      auto leaf = static_cast<Leaf<T> *>(ptr->children[l_idx].get());
      lvl = 0;
      l_idx = local_idx(idx, lvl);
      last_accessed_leaf.ptr = leaf;
      last_accessed_leaf.num = leaf_number(idx);
      last_accessed_leaf.read_only = read_only;
      return leaf->children[l_idx];
    } else {
      auto branch = static_cast<Branch<T> *>(ptr->children[l_idx].get());
      return get_some(branch, idx, lvl - 1, read_only);
    }
  }

  Branch<T> root;
  Leaf_ptr<T> last_accessed_leaf;
  uint32_t population;
  uint32_t levels;

public:
  explicit vector()
      : root(Branch<T>()), last_accessed_leaf(Leaf_ptr<T>()), population(0),
        levels(1) {}
  explicit vector(int size)
      : root(Branch<T>()), last_accessed_leaf(Leaf_ptr<T>()) {

    population = size;
    levels = get_min_level_count(size);

    // std::cout << "levels: " << levels << std::endl;

    auto last_idx = population - 1;
    construct_children(&root, last_idx, levels);
  }

  explicit vector(Branch<T> root, Leaf_ptr<T> lal, uint32_t pop, uint32_t lvls)
      : root(root), last_accessed_leaf(lal), population(pop), levels(lvls) {}

  vector<T> snapshot() {
    last_accessed_leaf.read_only = true;
    for (int i = 0; i < M; i++) {
      root.read_only[i] = true;
    }
    return vector(root, last_accessed_leaf, population, levels);
  }

  void set(const uint32_t idx, const T &some) {
    auto leaf_num = leaf_number(idx);
    if (leaf_num == last_accessed_leaf.num && !last_accessed_leaf.read_only) {
      // std::cout << "using quick access set" << std::endl;
      last_accessed_leaf.ptr->children[local_idx(idx, 0)] = some;
      // std::cout << "it was ok" << std::endl;
    } else {
      // std::cout << "goin to mutant upd: idx:" << idx << " lvls:  " << levels
      //<< " some:  " << some << std::endl;
      mutant_update(&root, idx, levels, some);
    }
  }

  void push_back(const T &some) {
    auto idx = population;
    population = population + 1;
    if (idx == (1 << (B * (levels + 1)))) {
      //  std::cout << "idx == (1 << (B * (levels + 1)))" << std::endl;
      root = grow(root);
      levels++;
    }
    if ((idx & (M - 1)) == 0) {
      //   std::cout << "idx & M" << std::endl;
      //   std::cout << "calling populate("
      //             << "&root " << idx << " " << levels << " "
      //             << "some" << std::endl;
      populate(&root, idx, levels, some);
    } else {
      set(idx, some);
    }
  }

  T get(const uint32_t idx) { return get_some(&root, idx, levels, false); }

  struct Proxy {
    vector &v;
    int idx;
    Proxy(vector &v, int idx) : v(v), idx(idx) {}

    operator T() {
      // std::cout << "reading\n"; return a.data[index];
      return v.get(idx);
    }

    // T &operator=(const T &some) {
    Proxy &operator=(const T &some) {
      v.set(idx, some);
      return *this;
    }
  };

  Proxy operator[](const int idx) { return Proxy(*this, idx); }
  uint32_t size() { return population; }
};
