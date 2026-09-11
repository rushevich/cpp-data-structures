#include "queue.hpp"
#include <cassert>
#include <iostream>
#include <type_traits>

using rushevich::queue;

void test_construction() {
  queue<int> q(5);
  assert(q.capacity() == 5);
  assert(q.size() == 0);
  assert(q.empty());
  assert(!q.full());
  assert(q.front() == nullptr);
}

void test_push_and_full() {
  queue<int> q(3);
  assert(q.try_push(1));
  assert(q.try_push(2));
  assert(q.try_push(3));
  assert(q.full());
  assert(q.size() == 3);
  assert(!q.try_push(4)); // full, should return false
  assert(q.size() == 3);  // unchanged
}

void test_pop_and_front() {
  queue<int> q(3);
  q.try_push(1);
  q.try_push(2);
  q.try_push(3);

  assert(*q.front() == 1);
  q.pop();
  assert(q.size() == 2);
  assert(*q.front() == 2);
  q.pop();
  assert(*q.front() == 3);
  q.pop();
  assert(q.empty());
  assert(q.front() == nullptr);
  // not calling pop() here since it asserts on empty by spec
}

void test_emplace() {
  struct Point {
    int x, y;
    Point(int a, int b) : x(a), y(b) {}
  };
  queue<Point> q(2);
  assert(q.try_emplace(1, 2));
  assert(q.try_emplace(3, 4));
  assert(!q.try_emplace(5, 6)); // full, should return false
  assert(q.front()->x == 1 && q.front()->y == 2);
}

void test_fifo_order() {
  queue<int> q(4);
  for (int i = 0; i < 4; ++i)
    assert(q.try_push(i));
  for (int i = 0; i < 4; ++i) {
    assert(*q.front() == i);
    q.pop();
  }
  assert(q.empty());
}

void test_wraparound() {
  queue<int> q(3);
  q.try_push(1);
  q.try_push(2);
  q.try_push(3);
  q.pop();               // remove 1, freeing a slot at the "back" of the buffer
  assert(q.try_push(4)); // should wrap around and succeed
  assert(*q.front() == 2);
  q.pop();
  assert(*q.front() == 3);
  q.pop();
  assert(*q.front() == 4);
  q.pop();
  assert(q.empty());
}

void test_zero_capacity_clamped_to_one() {
  queue<int> q(0);
  assert(q.capacity() == 1); // 0 is clamped up to 1
  assert(q.empty());
  assert(!q.full());
  assert(q.front() == nullptr);

  assert(q.try_push(1));
  assert(q.full());
  assert(!q.empty());
  assert(!q.try_push(2)); // already at capacity 1
  assert(*q.front() == 1);

  q.pop();
  assert(q.empty());
  assert(q.front() == nullptr);
}

void test_not_copyable_or_movable() {
  static_assert(!std::is_copy_constructible_v<queue<int>>);
  static_assert(!std::is_move_constructible_v<queue<int>>);
  static_assert(!std::is_copy_assignable_v<queue<int>>);
  static_assert(!std::is_move_assignable_v<queue<int>>);
}

int main() {
  test_construction();
  test_push_and_full();
  test_pop_and_front();
  test_emplace();
  test_fifo_order();
  test_wraparound();
  test_zero_capacity_clamped_to_one();
  test_not_copyable_or_movable();

  std::cout << "all tests passed\n";
  return 0;
}
