# Concurrent_Queue

Concurrent Queue benchmarks

A small suite to benchmark throughput and latency of several single-producer/single-consumer (SPSC) queue implementations in C++. 
The current fastest in my implementation is SPSCFastQueue. 

Motivation

Lock-free and wait-free queues behave very differently depending on the CPU, cache topology, and traffic pattern (bursty versus steady state). 
This repo aims to provide microbenchmarks that you can run locally. 

Currently implemented versions:

- SPSCFastQueue - fixed capacity ring buffer 
- Mutex based queue - classic lock based queue
- Boost lockfree queue - as a baseline - current SPSCFastQueue beats this queue with a good margin  

Build & Requirements

- C++17 or newer
- -pthread


Current performance 

```
ops total (GenSPSC Q for Test Struct): 46812631 
ops total (GenSPSC Q for int): 45871810 
ops total (Boost Q): 36101913 
ops total (Mutex Q): 16865823 
ops total (GenLocalHTSPSC Q for Test Struct): 65578759 
ops total (FastSPSC Q for Test Struct): 93073908 
All benchmarks have been run 
```

TODO: add latency numbers and clean up the throughput defintion

Design notes

SPSCFastQueue is an SPSC ring that synchronizes via cache-line-padded head/tail atomics, keeping the payload path free of atomics; in steady state it does one release store per side and touches acquire loads only near full/empty → typically the fastest. 

GenSPSCQueueLocalHT uses a per-slot version tag (0/1) as the handshake; it’s simpler to reason about and easier to extend with per-cell metadata, but it performs an acquire+release on every message and flips a tag inside the slot’s cache line, which can reduce throughput on modern cores.