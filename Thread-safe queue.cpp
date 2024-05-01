#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <functional>
#include <queue>
#include <atomic>

void func1() {
    std::cout << "working " << __FUNCTION__ << std::endl;
}

void func2() {
    std::cout << "working " << __FUNCTION__ << std::endl;

}

template<class T>
class safe_queue {
private:

    std::queue<std::function<void()>> WorkQueue;
    std::mutex workprog;
    std::atomic<bool> flag{ false };


public:

    void push( T(*f)()) {
        for (int i = 0; i < 5; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::lock_guard<std::mutex> Lockguard{ workprog };
            WorkQueue.push(f);
        }
       

    }

    void push2(T(*f)()) {
        for (int i = 0; i < 5; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::lock_guard<std::mutex> Lockguard{ workprog };
            WorkQueue.push(f);
        }
        flag = true;

    }

    void pops() {
      
        while (!flag) {
            std::lock_guard<std::mutex> Lockguard{ workprog };
            if (!WorkQueue.empty()) {
                auto task = WorkQueue.front();
                WorkQueue.pop();
                task();
            }
            else {
                std::this_thread::yield();
            }

        }
        std::lock_guard<std::mutex> Lockguard{ workprog };
        if (!WorkQueue.empty()) {
            auto task = WorkQueue.front();
            WorkQueue.pop();
            task();
        }
    }

};


class thread_pool {
private:
    std::vector<std::thread> t_vec;
    int maxThreadCount = std::thread::hardware_concurrency();
    safe_queue<void> s_q;
public:

    thread_pool() {
    
        for (int i = 0; i < maxThreadCount  ; i++) {

            t_vec.push_back(std::thread(&safe_queue<void>::pops, std::ref(s_q)));

        }

    }

   
    void work() {
        for (int i = 0; i < maxThreadCount ; i++) {
            t_vec[i].join();
        }

    }
    
    void submit(void (*f)()  ) {
        s_q.push(f);
    }

    void submit2(void (*f)()) {
        s_q.push2(f);
    }

};


int main()
{
    thread_pool t1;
    t1.submit(func1);
    t1.submit2(func2);
   
    
    t1.work();
    return 0;
}
