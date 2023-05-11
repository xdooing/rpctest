#pragma once
#include<queue>
#include<thread>
#include<mutex>
#include<condition_variable>




// 异步日志队列
template<typename T>
class LockQueue{
public:
    // 写日志进队列 --- 多个工作线程
    void Push(const T& data){
        std::lock_guard<std::mutex> locker(m_mutex);
        m_queue.push(data);
        m_cond.notify_one();
    }

    // IO输出日志 --- 一个线程
    T Pop(){
        std::unique_lock<std::mutex> locker(m_mutex);
        while(m_queue.empty()){
            m_cond.wait(locker);
        }
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};