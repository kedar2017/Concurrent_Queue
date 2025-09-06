#include "../concurrent_queue.h"
#include "thread"

#define CUSTOM_ASSERT(flag, msg) if (!flag) {std::cerr << "Assertion failed: " << msg << " \n"; }

void basic_test_case_1 () {
    SPSCQueue que(5);
    CUSTOM_ASSERT(que.push(1), "Did Push(1) succeed ?");
    CUSTOM_ASSERT(que.push(2), "Did Push(2) succeed ?");
    CUSTOM_ASSERT(que.push(3), "Did Push(3) succeed ?");
    CUSTOM_ASSERT(que.push(4), "Did Push(4) succeed ?");
    CUSTOM_ASSERT(!que.push(5), "Did Push(5) succeed ?");

    int head, val, counter = 0;
    while (counter < 4) {
        head = que.head.load(std::memory_order_acquire);
        val = que.arr[counter].element;
        CUSTOM_ASSERT(val == counter+1, "Value at index is unexpected");
        CUSTOM_ASSERT(que.deque(), "Deque failed");
        counter++;
    }
}

void basic_test_case_2 () {
    SPSCQueue que(6);
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
    SPSCQueue que(6);
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
    SPSCQueue que(100);
    int counter = 1000000;

    auto prodFunc = [&]() {
        for (int i = 0; i < counter; i++) {
            while (!que.push(i)) std::this_thread::yield();
        }
    };

    auto consFunc = [&]() {
        while (que.is_empty()) std::this_thread::yield();
        int expected = 0;
        while (expected < 1000000) {
            int popped_ele;
            if (!que.pop(popped_ele)) {
                std::this_thread::yield();
                continue;
            }
            CUSTOM_ASSERT((expected == popped_ele), "Peek value mismatch");
            ++expected;
        }
    };

    std::thread prodThread(prodFunc);
    std::thread consThread(consFunc);

    prodThread.join();
    consThread.join();
}

void gen_spsc_test_case_1 () {
    GenSPSCQueue que(100, 20);

    struct TestStruct {
        int a = 0;
        int b = 0;
        int c = 0;
    };

    TestStruct t1, t2, t3, t4, t5;
    TestStruct ret;

    CUSTOM_ASSERT(que.push(t1), "Did Push(1) succeed ?");
    CUSTOM_ASSERT(que.push(t2), "Did Push(2) succeed ?");
    CUSTOM_ASSERT(que.push(t3), "Did Push(3) succeed ?");
    CUSTOM_ASSERT(que.push(t4), "Did Push(4) succeed ?");
    CUSTOM_ASSERT(que.pop(ret), "Deque failed");
    CUSTOM_ASSERT(que.push(t5), "Did Push(5) succeed ?");
    CUSTOM_ASSERT((bool) (sizeof(TestStruct) < 14), "Struct too big");
}

void gen_spsc_test_case_2 () {
    GenSPSCQueue que(100, 20);

    struct TestStruct {
        int a = 0;
        int b = 0;
        int c = 0;
    };

    int counter = 1000000;

    auto prodFunc = [&]() {
        for (int i = 0; i < counter; i++) {
            TestStruct test;
            test.a = i;
            while (!que.push(test)) std::this_thread::yield();
        }
    };

    auto consFunc = [&]() {
        while (que.is_empty()) std::this_thread::yield();
        int expected = 0;
        while (expected < 1000000) {
            TestStruct test;
            if (!que.pop(test)) {
                std::this_thread::yield();
                continue;
            }
            CUSTOM_ASSERT((expected == test.a), "Peek value mismatch");
            ++expected;
        }
    };

    std::thread prodThread(prodFunc);
    std::thread consThread(consFunc);

    prodThread.join();
    consThread.join();

}

int main() {
    basic_test_case_1();
    basic_test_case_2();
    basic_test_case_3();
    spsc_test_case_1();
    gen_spsc_test_case_1();
    gen_spsc_test_case_2();
    std::cout << "All SPSCQueue tests passed.\n";
    return 0;
}