# Concurrent_Queue

> Concurrent queue microbenchmark suite – currently focused on single-producer / single-consumer (SPSC) queues in modern C++.

This repository contains a small, focused benchmark harness for comparing different SPSC queue implementations — from a simple mutex-based queue to custom lock-free ring buffers and Boost’s lock-free queue.

It’s both a **learning project** (atomics, cache behavior, lock-free vs lock-based) and a **reusable microbenchmark** you can run on your own hardware.

---

## Table of contents

- [Motivation](#motivation)  
- [Implemented queues](#implemented-queues)  
- [Repository layout](#repository-layout)  
- [Building & running](#building--running)  
- [Example usage](#example-usage)  
- [Sample performance results](#sample-performance-results)  
- [Design notes](#design-notes)  
- [Future work](#future-work)  

---

## Motivation

Lock-free and wait-free queues behave very differently depending on:

- CPU microarchitecture  
- Cache topology / NUMA layout  
- Traffic pattern (steady state vs bursty)  

Rather than relying on generic “X is faster than Y” claims, this repo provides microbenchmarks you can run locally to:

- Compare **simple mutex-based queues** against **lock-free SPSC queues**
- See the impact of **cache-line padding**, **per-slot metadata**, and **atomics**
- Get an intuition for **throughput vs latency** trade-offs in real systems

---

## Implemented queues

Currently implemented queue variants:

| Queue                       | Style                      | Progress model      | Notes |
|----------------------------|----------------------------|---------------------|-------|
| `SPSCFastQueue`            | Fixed-capacity ring buffer | SPSC, lock-free-ish | Cache-line-padded head/tail atomics, payload path free of atomics in steady state. Typically the fastest implementation. |
| `GenSPSCQueue`             | Generic SPSC queue         | SPSC                | More general design; configurable block size and payload type. |
| `GenSPSCQueueLocalHT`      | Per-slot version tag queue | SPSC                | Per-slot 0/1 “handshake” tag; easy to reason about / extend with metadata. |
| `Boost::lockfree::queue`   | External baseline          | Lock-free           | Used as a reference implementation and baseline. |
| `MutexQueue` (mutex-based) | `std::mutex` + container   | SPSC                | Simple lock-based baseline to compare against. |

> Note: all queues in this repo are SPSC – one dedicated producer thread and one dedicated consumer thread.

---

## Repository layout

- `concurrent_queue.h`  
  Core SPSC queue implementations (`SPSCFastQueue`, `GenSPSCQueue`, `GenSPSCQueueLocalHT`, mutex-based queue, thin wrapper around Boost).

- `benchmark/`  
  Benchmark harnesses for measuring throughput for different queue types and payloads (`int`, custom `TestStruct`, etc.).

- `tests/`  
  Small correctness tests / sanity checks for the queue APIs.

---

## Building & running

### Requirements

- C++17 or newer
- POSIX threads (`-pthread`)
- Boost (for the Boost lock-free queue baseline)

### Build

Example using `g++`:

```bash
git clone https://github.com/kedar2017/Concurrent_Queue.git
cd Concurrent_Queue

# Basic benchmark build (adjust *.cpp as needed)
g++ -std=c++17 -O3 -pthread -I. benchmark/*.cpp -o spsc_bench

```

## Run benchmarks 

```./spsc_bench
```

The benchmark executable prints throughput numbers (operations completed) for each queue implementation / payload combination over a fixed test window.

## Example usage 

Below is a minimal sketch of using an SPSC queue

```
#include "concurrent_queue.h"
#include <thread>
#include <iostream>

int main() {
    constexpr std::size_t capacity = 1 << 16;

    // Example: high-throughput SPSC ring buffer
    SPSCFastQueue<int> q(capacity);

    constexpr int N = 1'000'000;

    std::thread producer([&] {
        for (int i = 0; i < N; ++i) {
            while (!q.try_push(i)) {
                // queue full → busy-wait / backoff
            }
        }
    });

    std::thread consumer([&] {
        int value = 0;
        for (int i = 0; i < N; ++i) {
            while (!q.try_pop(value)) {
                // queue empty → busy-wait / backoff
            }
            // process(value);
        }
    });

    producer.join();
    consumer.join();

    std::cout << "Done\n";
    return 0;
}
```

## Sample benchmark results (x86, higher is better)

Example results from one run on my development machine:

| Queue variant                       | Payload       | Ops total    | × vs `Mutex Q` | × vs `Boost::lockfree::queue` |
|------------------------------------|--------------|-------------:|---------------:|-------------------------------:|
| `Mutex Q`                          | `int`        | 16,865,823   | 1.00×          | 0.47×                          |
| `Boost::lockfree::queue`           | `int`        | 36,101,913   | 2.14×          | 1.00×                          |
| `GenSPSC Q`                        | `int`        | 45,871,810   | 2.72×          | 1.27×                          |
| `GenSPSC Q`                        | `TestStruct` | 46,812,631   | 2.78×          | 1.30×                          |
| `GenLocalHTSPSC Q`                 | `TestStruct` | 65,578,759   | 3.89×          | 1.82×                          |
| `FastSPSC Q`                       | `TestStruct` | 93,073,908   | **5.52×**      | **2.58×**                      |

> “Ops total” = number of successful push/pop pairs completed during the fixed benchmark window.  
> These numbers are from a single x86 machine and are meant as a comparative snapshot, not absolute performance claims.

These numbers are hardware- and config-specific. The main goal is to:

- Compare multiple designs on the same machine
- See how design choices affect throughput

Planned additions:

- Latency stats (min/avg/p99)
- Clearer “throughput” definition and reporting
- Parameter sweeps for message sizes / burst sizes


## Design notes

`SPSCFastQueue`

- Bounded SPSC ring buffer with cache-line-padded head / tail atomics.
- Payload slots themselves are non-atomic in steady state.
- This tends to make it the top performer in throughput benchmarks.

`GenSPSCQueueLocalHT`

- Uses per-slot version tags (0 / 1) as the handshake mechanism.
- Very clear correctness story (producer and consumer each check/flip tags).
- Easy to extend with additional per-slot metadata.
- But: incurs an acquire + release on each message and toggles metadata in the same cache line as payload, which can hurt throughput on modern cores.

`Baseline designs`

- A simple mutex-based queue provides an intuitive, readable reference.
- Boost’s lock-free queue offers a widely-used, production-quality baseline.


## Future work 

Some ideas for extending / polishing this repo:

- Better separation between queue implementations
- Latency histograms and p99/p999 reporting
- NUMA-aware experiments (CPU pinning, cross-socket tests)
- MPMC variants and comparison against SPSC designs
