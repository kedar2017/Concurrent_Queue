#include <climits>

class Block {
public:
    int element = INT_MIN;
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