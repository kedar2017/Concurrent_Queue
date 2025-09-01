#include "../concurrent_queue.h"

#define CUSTOM_ASSERT(flag, msg) if (!flag) {std::cerr << "Assertion failed: " << msg << " \n"; }

void basic_test_case_1 () {
    SPSCQueue que(5);
    CUSTOM_ASSERT(que.push(1), "Did Push(1) succeed ?");
    CUSTOM_ASSERT(que.push(2), "Did Push(2) succeed ?");
    CUSTOM_ASSERT(que.push(3), "Did Push(3) succeed ?");
    CUSTOM_ASSERT(que.push(4), "Did Push(4) succeed ?");
    CUSTOM_ASSERT(que.push(5), "Did Push(5) succeed ?");

    int head, val, counter = 0;
    while (counter < 5) {
        head = que.head.load(std::memory_order_acquire);
        val = que.arr[counter].element;
        CUSTOM_ASSERT(val == counter+1, "Value at index is unexpected");
        CUSTOM_ASSERT(que.deque(), "Deque failed");
        counter++;
    }
}

void basic_test_case_2 () {
    SPSCQueue que(5);
    CUSTOM_ASSERT(que.push(1), "Did Push(1) succeed ?");
    CUSTOM_ASSERT(que.push(2), "Did Push(2) succeed ?");
    CUSTOM_ASSERT(que.push(3), "Did Push(3) succeed ?");
    CUSTOM_ASSERT(que.push(4), "Did Push(4) succeed ?");
    CUSTOM_ASSERT(que.push(5), "Did Push(5) succeed ?");
    CUSTOM_ASSERT(que.deque(), "Deque failed");
    CUSTOM_ASSERT(que.deque(), "Deque failed");

    int head, val, counter = 0;

    while (counter < 3) {
        head = que.head.load(std::memory_order_acquire);
        val = que.arr[head].element;
        CUSTOM_ASSERT(val == counter+1, "Value at index is unexpected");
        CUSTOM_ASSERT(que.deque(), "Deque failed");
        counter++;
    }

    CUSTOM_ASSERT(!que.deque(), "Deque failed");    
}

void basic_test_case_3 () {
    SPSCQueue que(5);
    CUSTOM_ASSERT(que.push(1), "Did Push(1) succeed ?");
    CUSTOM_ASSERT(que.push(2), "Did Push(2) succeed ?");
    CUSTOM_ASSERT(que.push(3), "Did Push(3) succeed ?");
    CUSTOM_ASSERT(que.push(4), "Did Push(4) succeed ?");
    CUSTOM_ASSERT(que.push(5), "Did Push(5) succeed ?");
    CUSTOM_ASSERT(que.deque(), "Deque failed");
    CUSTOM_ASSERT(que.deque(), "Deque failed");
    CUSTOM_ASSERT(que.deque(), "Deque failed");
    CUSTOM_ASSERT(que.deque(), "Deque failed");
    CUSTOM_ASSERT(que.push(6), "Did Push(6) succeed ?");
    CUSTOM_ASSERT(que.deque(), "Deque failed");

    int head = que.head.load(std::memory_order_acquire);
    int val = que.arr[head].element;
    CUSTOM_ASSERT((val == 6), "Value at index is unexpected");
    CUSTOM_ASSERT(que.deque(), "Deque failed");

    CUSTOM_ASSERT(que.push(7), "Did Push(7) succeed ?");
    head = que.head.load(std::memory_order_acquire);
    val = que.arr[head].element;
    CUSTOM_ASSERT((val == 7), "Value at index is unexpected");
}

void spsc_test_case_1 () {
    // TODO: will need some changes to the SPSC queue
}

int main() {
    basic_test_case_1();
    basic_test_case_2();
    basic_test_case_3();
    std::cout << "All SPSCQueue tests passed.\n";
    return 0;
}