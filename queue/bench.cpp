#include "queue.hpp"
#include <cassert>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <print>
#include <pthread.h>
#include <queue>
#include <sched.h>
#include <system_error>

namespace cr = std::chrono;

class Timer {
  enum class Status { stopped, running };

public:
  explicit Timer(std::string_view name) : _name{name} {}

  void start() { _start = clock::now(); }

  auto end() {
    const auto runtime = clock::now() - _start;
    _lastResult = runtime;
    return _lastResult.count();
  }

  using clock = cr::steady_clock;

  void dumpResult() { std::println("{}: {}ns", _name, _lastResult); }

private:
  cr::time_point<clock> _start;
  cr::nanoseconds _lastResult;
  Status _current;
  std::string _name;
};

// I stole this from rigtorp
void pinThread(int cpu) {
  if (cpu < 0) {
    return;
  }
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu, &cpuset);
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) ==
      -1) {
    throw std::system_error(errno, std::system_category(),
                            "Failed to set thread affinity");
  }
}

const char *end(const char *begin) {
  assert(begin != nullptr);
  while (*begin != '\0') {
    ++begin;
  }
  return begin;
}

namespace {
constexpr auto queueSize = 10'000'000UZ;
constexpr auto numIterations = 10'000'000LL;
} // namespace

int main(int argc, char **argv) {
  int cpu{};
  auto [ptr, res] = std::from_chars(argv[1], end(argv[1]), cpu);
  if (res != std::errc()) {
    throw std::runtime_error("Parsing failure");
  }
  pinThread(cpu);

  {
    using namespace rushevich;
    Timer t{"rushevich::queue"};
    queue<int> q(queueSize);
    t.start();
    for (int i{0}; i < numIterations; ++i) {
      q.try_emplace(i);
    }
    const auto insert_time = t.end();

    t.start();
    for (int i{0}; i < numIterations; ++i) {
      q.pop();
    }
    const auto pop_time = t.end();

    std::println(
        "rushevich::queue results\n---------------------\nTime to insert {0} "
        "elements: {1}\nInsertion ops/us: {3}\n---------------------\nTime to "
        "pop "
        "{0} elements: {2}\nPop ops/us: {4}",
        queueSize, insert_time, pop_time, numIterations * 1'000 / insert_time,
        numIterations * 1'000 / pop_time);
  }

  {
    using namespace std;
    Timer t{"std::queue"};
    queue<int> q;
    t.start();
    for (int i{0}; i < numIterations; ++i) {
      q.emplace(i);
    }
    const auto insert_time = t.end();

    t.start();
    for (int i{0}; i < numIterations; ++i) {
      q.pop();
    }
    const auto pop_time = t.end();

    std::println(
        "std::queue results\n---------------------\nTime to insert {0} "
        "elements: {1}\nInsertion ops/us: {3}\n---------------------\nTime to "
        "pop "
        "{0} elements: {2}\nPop ops/us: {4}",
        queueSize, insert_time, pop_time, numIterations * 1'000 / insert_time,
        numIterations * 1'000 / pop_time);
  }
}
