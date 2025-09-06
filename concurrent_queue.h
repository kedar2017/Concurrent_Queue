#include <climits>
#include <atomic>
#include <ostream>
#include <iostream>
#include <new>
#include <cstring>

class Block {
public:
    int element;
};

class GenBlock {
public:
    uint8_t* buffer;
    int version;
};

class BasicQueue {
public:
    explicit BasicQueue (int cap = 5)
        : capacity(cap), arr(new Block[cap]) {}

    ~BasicQueue() { delete[] arr; }
    
    int capacity;
    Block* arr;
    int head = 0;
    int tail = 0;

    bool push (int x) {
        if (is_full()) {
            std::cout << "Queue is full. Try later" << std::endl;
            return false;
        }
        arr[tail].element = x;
        tail = tick(tail);
        return true;
    }

    bool deque () {
        if (is_empty()) {
            std::cout << "Queue is empty. Try later" << std::endl;
            return false;
        }
        arr[head].element = INT_MIN;
        head = tick(head);
        return true;
    }

    bool is_full () {
        return (tail == head) && (arr[tail].element != INT_MIN);
    }

    bool is_empty () {
        return (tail == head) && (arr[tail].element == INT_MIN);
    }

    int tick (int x) {
        if (x == capacity - 1) {
            x = 0;
        } else {
            x++;
        }
        return x;
    }
};

class SPSCQueue {
public:
    explicit SPSCQueue (int cap = 5)
        : capacity(cap), arr(new Block[cap]) {}

    ~SPSCQueue() { delete[] arr; }
    
    int capacity;
    Block* arr;
    std::atomic<int> head{0};
    std::atomic<int> tail{0};

    bool push (int x) {
        int head_temp = head.load(std::memory_order_relaxed);
        int tail_temp = tail.load(std::memory_order_acquire);
        int temp = tick(tail_temp);

        if (temp == head_temp) return false;

        arr[tail_temp].element = x;
        tail.store(temp, std::memory_order_release);
        return true;
    }

    bool deque () {
        int head_temp = head.load(std::memory_order_acquire);
        int tail_temp = tail.load(std::memory_order_relaxed);
        
        if (head_temp == tail_temp) return false;
        
        head.store(tick(head_temp), std::memory_order_release);
        return true;
    }

    bool is_empty () {
        int head_temp = head.load(std::memory_order_acquire);
        int tail_temp = tail.load(std::memory_order_acquire);

        if (head_temp == tail_temp) {
            return true;
        } else {
            return false;
        }
    }

    bool pop (int& popped_ele) {
        int head_temp = head.load(std::memory_order_acquire);
        int tail_temp = tail.load(std::memory_order_relaxed);
        
        if (head_temp == tail_temp) return false;
        
        popped_ele = arr[head_temp].element;
        head.store(tick(head_temp), std::memory_order_release);
        return true;
    }

    int tick (int x) {
        if (x == capacity - 1) {
            x = 0;
        } else {
            x++;
        }
        return x;
    }
};

class GenSPSCQueue {
public:
    
    GenSPSCQueue(int capacity, int size) {
        cap = capacity;
        block_size = size;
        arr = new GenBlock[capacity];
        
        for (int i = 0; i < capacity; i++) {
            arr[i].version = 1;
            arr[i].buffer = new uint8_t[block_size];
        }
    }

    ~GenSPSCQueue() { 
        for (int i = 0; i < cap; i++) {
            delete[] arr[i].buffer;
        }
        delete[] arr;
    }

    template<class T>
    bool push (const T& v) {
        int head_temp = head.load(std::memory_order_relaxed);
        int tail_temp = tail.load(std::memory_order_acquire);
        int temp = tick(tail_temp);

        if (temp == head_temp) return false;

        memcpy(arr[tail_temp].buffer, &v, sizeof(T));

        tail.store(temp, std::memory_order_release);
        return true;
    }

    template<class T>
    bool pop (T& v) {
        int head_temp = head.load(std::memory_order_acquire);
        int tail_temp = tail.load(std::memory_order_relaxed);
        
        if (head_temp == tail_temp) return false;
        
        memcpy(&v, arr[head_temp].buffer, sizeof(T));

        head.store(tick(head_temp), std::memory_order_release);
        return true;
    }

    bool is_empty () {
        int head_temp = head.load(std::memory_order_acquire);
        int tail_temp = tail.load(std::memory_order_acquire);

        if (head_temp == tail_temp) {
            return true;
        } else {
            return false;
        }
    }

    int tick (int x) {
        if (x == cap - 1) {
            x = 0;
        } else {
            x++;
        }
        return x;
    }

    GenBlock* arr;
    int cap;
    int block_size;
    std::atomic<int> head{0};
    std::atomic<int> tail{0};
};

