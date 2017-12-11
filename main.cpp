#include "vector.hpp"
#include <chrono>
#include <iostream>
#include <vector>

static const int s = 1000000;

template <typename Fn> double runtime(Fn f, int rep) {

  double sum = 0;

  for (int i = 0; i < rep; i++) {
    std::chrono::high_resolution_clock::time_point t1 =
        std::chrono::high_resolution_clock::now();
    f();
    std::chrono::high_resolution_clock::time_point t2 =
        std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    sum += duration;
  }
  return sum / rep;
}

using std::pair;

void function1() {
  auto v = vector<pair<double, double>>();
  for (int i = 0; i < s; i++) {
    v.push_back(make_pair(1.123, 2.2345));
  }
}

void function2() {
  auto v = std::vector<pair<double, double>>();
  for (int i = 0; i < s; i++) {
    v.push_back(make_pair(1.123, 2.2345));
  }
}

void subscript() {
  auto v = vector<pair<double, double>>(s);
  for (int i = 0; i < s; i++) {
    v[i] = (make_pair(1.123, 2.2345));
  }
}

void set() {
  auto v = vector<pair<double, double>>(s);
  for (int i = 0; i < s; i++) {
    v.set(i, make_pair(1.123, 2.2345));
  }
}

void std_subscript() {
  auto v = std::vector<pair<double, double>>(s);
  for (int i = 0; i < s; i++) {
    v[i] = (make_pair(1.123, 2.2345));
  }
}

void snap() {
  auto v = vector<pair<double, double>>(s);
  auto v2 = v.snapshot();
  for (int i = 0; i < s; i++) {
    v2.set(i, make_pair(1.123, 2.2345));
  }
}

void snap2() {
  auto v = vector<bool>(s);
  auto v2 = v.snapshot();
  for (int i = 0; i < s; i++) {
    v2.set(i, true);
  }
}

void sub2() {
  auto v = vector<bool>(s);
  for (int i = 0; i < s; i++) {
    v[i] = true;
  }
}

void snap3() {
  auto v = vector<long long>();
  for (long long i = 0; i < s; i++) {
    v.push_back(i);
  }
  auto v2 = v.snapshot();
  for (long long i = 0; i < s; i++) {
    v2[i] = 4 * i;
  }
}

void std_snap3() {
  auto v = std::vector<long long>();
  for (long long i = 0; i < s; i++) {
    v.push_back(i);
  }

  auto v2 = std::vector<long long>();

  for (long long i = 0; i < v.size(); i++) {
    v2.push_back(v[i]);
  }

  for (long long i = 0; i < s; i++) {
    v2[i] = 4 * i;
  }
}

template <typename T> void print(T some) { std::cout << some << std::endl; }

using std::cout;
using std::endl;
int main() {
  auto r1 = runtime(function1, 50);
  auto r2 = runtime(function2, 50);
  cout << "run time for push_back(),\n<pair<double,double>>\n"
       << "vector / std::vector = " << (r1 / r2) << endl
       << endl
       << endl;
  auto std_sub = runtime(std_subscript, 50);

  auto sub = runtime(subscript, 50);

  // auto sett = runtime(set, 50);
  // sub / sett);

  cout << "run time for subscript operator,\n<pair<double, double>>\n"
       << "vector / std::vector = " << sub / std_sub << endl
       << endl;

  auto snapt = runtime(snap, 50);

  cout << "run time after snapshop vs nosnap,\n"
       << "using pair<double, double>\n:"
       << "snap / no_snap = " << snapt / sub << endl
       << endl;

  auto snapt2 = runtime(snap2, 50);
  auto subs2 = runtime(sub2, 50);

  cout << "run time after snapshop vs nosnap,\n"
       << "using < bool >\n"
       << "snap / no_snap = " << snapt2 / subs2 << endl
       << endl;

  auto snap3t = runtime(snap3, 50);
  auto std_snap3t = runtime(std_snap3, 50);

  cout << "run time after snapshop vs taking a copy of std::vector,\n"
       << "using < long long >\n"
       << "snap / std_copy = " << (snap3t / std_snap3t) << endl
       << endl;
}
