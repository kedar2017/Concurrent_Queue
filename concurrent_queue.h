#include <climits>
#include <atomic>
#include <ostream>
#include <iostream>
#include <new>
#include <cstring>
#include <mutex>
#include <queue>
#include <condition_variable>

class Block {
public:
    int element;
};

class GenBlock {
public:
    uint8_t* buffer;
    int version;
};

class GenBlockLocalVar {
public:
    uint8_t* buffer;
    std::atomic<int> version;
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

template <typename T>
class MutexQueue {
public:
    explicit MutexQueue (int cap = 5)
        : capacity(cap), que(cap) {}

    bool push (T item) {
        std::unique_lock<std::mutex> lock(q_mutex);
        if (size == capacity) return false;
        que[tail] = item;
        tail = (tail + 1) % capacity;
        size++;
        return true;
    }

    bool pop (T& item) {
        std::unique_lock<std::mutex> lock(q_mutex);
        if (size == 0) return false;
        item = que[head];
        head = (head + 1) % capacity;
        size--;
        return true;
    }

    bool empty () {
        std::unique_lock<std::mutex> lock(q_mutex);
        return (size == 0);
    }
    std::vector<T> que;
    int head = 0, tail = 0, size = 0;
    std::mutex q_mutex;
    int capacity;
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

template <typename T>
class GenSPSCQueue {
public:
    
    GenSPSCQueue(int capacity) {
        cap = capacity;
        block_size = sizeof(T);
        arr = new GenBlock[capacity];
        pre_alloc = new uint8_t[block_size * capacity];
        
        for (int i = 0; i < capacity; i++) {
            arr[i].version = 1;
            arr[i].buffer = pre_alloc + block_size * i;
        }
    }

    ~GenSPSCQueue() { 
        delete[] pre_alloc;
        delete[] arr;
    }

    bool push (const T& v) {
        int head_temp = head.load(std::memory_order_relaxed);
        int tail_temp = tail.load(std::memory_order_acquire);
        int temp = tick(tail_temp);

        if (temp == head_temp) return false;

        memcpy(arr[tail_temp].buffer, &v, sizeof(T));

        tail.store(temp, std::memory_order_release);
        return true;
    }

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
    uint8_t* pre_alloc;
    int cap;
    int block_size;
    std::atomic<int> head{0};
    std::atomic<int> tail{0};
};


template <typename T>
class GenSPSCQueueLocalHT {
public:
    
    GenSPSCQueueLocalHT (int capacity) {
        cap = capacity;
        block_size = sizeof(T);
        arr = new GenBlockLocalVar[capacity];
        pre_alloc = new uint8_t[block_size * capacity];
        
        for (int i = 0; i < capacity; i++) {
            arr[i].version = 0;
            arr[i].buffer = pre_alloc + block_size * i;
        }
    }

    ~GenSPSCQueueLocalHT () { 
        delete[] pre_alloc;
        delete[] arr;
    }

    bool push (const T& v) {
        int version_curr = arr[tail].version.load(std::memory_order_acquire);
        if (version_curr == 1) return false;

        memcpy(arr[tail].buffer, &v, sizeof(T));
        arr[tail].version.store(1, std::memory_order_release);
        tail = tick(tail);
        return true;
    }

    bool pop (T& v) {
        int version_curr = arr[head].version.load(std::memory_order_acquire);

        if (version_curr == 0) return false;
        
        memcpy(&v, arr[head].buffer, sizeof(T));

        arr[head].version.store(0, std::memory_order_release);
        head = tick(head);
        return true;
    }

    bool is_empty () {
        int version_curr = arr[head].version.load(std::memory_order_acquire);
        return version_curr == 0;
    }

    int tick (int x) {
        if (x == cap - 1) {
            x = 0;
        } else {
            x++;
        }
        return x;
    }

    GenBlockLocalVar* arr;
    uint8_t* pre_alloc;
    int cap;
    int block_size;
    int head{0};
    int tail{0};
};
