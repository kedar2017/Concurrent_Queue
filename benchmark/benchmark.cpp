#include <boost/lockfree/spsc_queue.hpp>
#include "thread"
#include "../tests/spsc_test.h"
#include <chrono>

using clock_tt = std::chrono::steady_clock;

void throughtput_genspscq_benchmark () {

    struct TestStruct {
        int a = 0;
        int b = 0;
        int c = 0;
    };
    std::atomic<bool> start{false}, stop{false};
    uint64_t produced = 0;
    uint64_t consumed = 0;

    GenSPSCQueue que(1000, 12);

    auto prodFunc = [&]() {
        while (!start.load(std::memory_order_acquire)) {}
        while (!stop.load(std::memory_order_relaxed)) {
            TestStruct test;
            while (!que.push(test)) std::this_thread::yield();
            produced++;
        }
    };

    auto consFunc = [&]() {
        while (!start.load(std::memory_order_acquire)) {}
        while (que.is_empty()) std::this_thread::yield();
        while (!stop.load(std::memory_order_relaxed)) {
            TestStruct test;
            if (!que.pop(test)) {
                std::this_thread::yield();
                continue;
            }
            consumed++;
        }
    };

    std::thread prodThread(prodFunc);
    std::thread consThread(consFunc);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    start.store(true, std::memory_order_release);
    auto t0 = clock_tt::now();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    stop.store(true, std::memory_order_release);
    prodThread.join();
    consThread.join();

    auto t1 = clock_tt::now();

    std::cout << "ops total (GenSPSC Q): " << consumed << " \n";
}

void throughtput_boostq_benchmark () {

    struct TestStruct {
        int a = 0;
        int b = 0;
        int c = 0;
    };
    std::atomic<bool> start{false}, stop{false};
    uint64_t produced = 0;
    uint64_t consumed = 0;

    boost::lockfree::spsc_queue<TestStruct, boost::lockfree::capacity<1000>> que;

    auto prodFunc = [&]() {
        while (!start.load(std::memory_order_acquire)) {}
        while (!stop.load(std::memory_order_relaxed)) {
            TestStruct test;
            while (!que.push(test)) std::this_thread::yield();
            produced++;
        }
    };

    auto consFunc = [&]() {
        while (!start.load(std::memory_order_acquire)) {}
        while (que.empty()) std::this_thread::yield();
        while (!stop.load(std::memory_order_relaxed)) {
            TestStruct test;
            if (!que.pop(test)) {
                std::this_thread::yield();
                continue;
            }
            consumed++;
        }
    };

    std::thread prodThread(prodFunc);
    std::thread consThread(consFunc);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    start.store(true, std::memory_order_release);
    auto t0 = clock_tt::now();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    stop.store(true, std::memory_order_release);
    prodThread.join();
    consThread.join();

    auto t1 = clock_tt::now();

    std::cout << "ops total (Boost Q): " << consumed << " \n";
}

int main() {
    
    basic_test_case_1();
    basic_test_case_2();
    basic_test_case_3();
    spsc_test_case_1();
    gen_spsc_test_case_1();
    gen_spsc_test_case_2();
    std::cout << "All SPSCQueue tests passed.\n";
    
    throughtput_genspscq_benchmark();
    throughtput_boostq_benchmark();
    std::cout << "All benchmarks have been run \n";
    return 0;
}
