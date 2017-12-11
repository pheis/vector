#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "vector.hpp"
#include <iostream>
#include <vector>

// TODO randomized testing

TEST_CASE("Integers are inserted and read", "[vector]") {

  int size_for_vector = 1000000;
  std::cout << "vector creation: " << std::endl;
  auto v1 = vector<int>(size_for_vector);
  auto v2 = std::vector<int>(size_for_vector);

  for (int i = 0; i < size_for_vector; i++) {
    v1.set(i, i);
    v2[i] = i;
  }

  REQUIRE(v1.get(size_for_vector - 1) == size_for_vector - 1);
  REQUIRE(v1.get(1) == 1);
  REQUIRE(v1.get(0) == 0);
  REQUIRE(v1.get(31) == 31);
  REQUIRE(v1.get(1024) == 1024);

  for (int i = 0; i < size_for_vector; i++) {
    REQUIRE(v1.get(i) == v2[i]);
  }
}

// TODO do more snapshot testing
TEST_CASE("Snapshot testing", "[vector]") {
  int size = 100000;

  auto v1 = vector<int>(size);
  for (int i = 0; i < size; i++) {
    v1.set(i, i);
  }

  auto v2 = v1.snapshot();
  for (int i = 0; i < size; i++) {
    v2.set(i, 2 * i);
  }

  REQUIRE(v1.get(size - 1) == size - 1);
  REQUIRE(v1.get(1) == 1);
  REQUIRE(v1.get(0) == 0);
  REQUIRE(v1.get(31) == 31);
  REQUIRE(v1.get(1024) == 1024);

  REQUIRE(v2.get(size - 1) == 2 * (size - 1));
  REQUIRE(v2.get(1) == 2 * 1);
  REQUIRE(v2.get(0) == 0);
  REQUIRE(v2.get(31) == 2 * 31);
  REQUIRE(v2.get(1024) == 2 * 1024);
}

TEST_CASE("push_back", "[vector]") {
  auto v1 = vector<int>();
  int size = 10000;
  for (int i = 0; i < size; i++) {
    v1.push_back(i);
  }
  REQUIRE(v1.get(size - 1) == size - 1);
  REQUIRE(v1.get(1) == 1);
  REQUIRE(v1.get(0) == 0);
  REQUIRE(v1.get(31) == 31);
  REQUIRE(v1.get(1024) == 1024);
}

TEST_CASE("subscript operator", "[vector]") {
  int size = 10000;
  auto v1 = vector<int>(size);
  for (int i = 0; i < size; i++) {
    v1[i] = i;
  }
  REQUIRE(v1.get(size - 1) == size - 1);
  REQUIRE(v1.get(1) == 1);
  REQUIRE(v1.get(0) == 0);
  REQUIRE(v1.get(31) == 31);
  REQUIRE(v1.get(1024) == 1024);
}
