#include "unique_ptr.hpp"

#include <cassert>
#include <format>
#include <print>

namespace {
constexpr int magic_number = 68;
template <typename T> struct StatefulDeleter {
    void operator()(T* ptr) const {
        std::println("Magical deleter number: {}", number);
        delete ptr;
    }
    int number = magic_number;
};

} // namespace

void test_size() {
    auto int_unique = rushevich::make_unique<int>(5 + 5);
    assert(sizeof(decltype(int_unique)) == 8 && "rushevich::unique_ptr is too big!");
    struct BigType {
        long long a {};
        unsigned long long b {};
    };

    auto big_unique
        = rushevich::unique_ptr<BigType, StatefulDeleter<BigType>> { new BigType { 5LL, 5ULL } };
    std::println("The ptr is actually {} bytes big", sizeof(decltype(big_unique)));
    assert(sizeof(decltype(big_unique)) == 16
           && sizeof(StatefulDeleter<BigType>)
                  == 4); // this is 16 bytes large despite the deleter being only 4 because the
                         // alignment of the unique_ptr is 8 bytes
    std::println("Size test passed");
}

int main() {
    std::println("Hello World!");
    test_size();
    return 0;
}
