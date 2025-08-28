#include <iostream>
#include <string>
#include "concurrent_queue.h"

int main() {
    BasicQueue que;
    que.push(2);
    que.push(2);
    que.push(1);
    que.push(3);
    que.push(4);
    que.push(10);
    que.push(10);
    que.push(10);
    que.push(10);
    que.deque();
    que.deque();
    
    for (int i = 0; i < 5; i++) {
        std::cout << "Values: " << que.arr[i].element << std::endl;
    }
}