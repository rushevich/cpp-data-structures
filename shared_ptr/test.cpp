#include "shared_ptr.hpp"

#include <cassert>
#include <print>

using namespace rushevich;

void test1() {
    shared_ptr<int> masta { new int { 5 } };
    assert(masta.refCount() == 1 && "Ref count: expected 1");
    {
        shared_ptr<int> ptr1 { masta };
        assert(masta.refCount() == 2 && "Ref count: expected 2");
        {
            shared_ptr<int> ptr2 { ptr1 };
            assert(masta.refCount() == 3 && "Ref count: expected 3");
        }
        assert(masta.refCount() == 2 && "Ref count: expected 2");
    }
    assert(masta.refCount() == 1 && "Ref count: expected 1");
}

void test2() {
    shared_ptr<int> masta;
    masta = new int { 5 };
    assert(masta.refCount() == 1 && "Ref count: expected 1");

    shared_ptr<int> newMasta = std::move(masta);
    assert(newMasta.refCount() == 1 && "Ref count: expected 1");

    shared_ptr<int> toMove = new int { 10 };
    assert(toMove.refCount() == 1 && "Ref count: expected 1");
    toMove = std::move(newMasta);
    assert(toMove.refCount() == 1 && "Ref count: expected 1");
}

void test3() {
    struct Basic {
        double x;
        double y;
    };

    shared_ptr<Basic> objPtr { new Basic { .x = 5.0, .y = 7.5 } };
    assert(objPtr->x == 5.0 && (*objPtr).y == 7.5);
}

int main() {
    test1();
    test2();
    test3();

    return 0;
}
