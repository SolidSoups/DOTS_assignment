#include "ThreadPool.h"
#include <iostream>

ThreadPool::ThreadPool()
  : num_threads(std::thread::hardware_concurrency())

{
  // RaII
  std::cout << "[ThreadPool] Creating Threads" << "\n";
  for(uint32_t i = 0; i < num_threads; ++i){
    m_threads.emplace_back(std::thread(&ThreadPool::threadLoop, this));
  }
  std::cout << "[ThreadPool] Threads Created" << "\n";
}

ThreadPool::~ThreadPool(){
  stop();
}

void ThreadPool::threadLoop(){
  while(true){
    std::function<void()> job;
    {
      // lock this scope, other thread will wait
      std::unique_lock<std::mutex> lock(m_queue_mutex);
      // unlock mutex and wait for new work or termination,
      // else lock mutex and continue
      m_queue_condition.wait(lock, [this]{
        return !m_jobs.empty() || shouldTerminate;
      });
      if(shouldTerminate){
        return;
      }
      // get the next job
      job = m_jobs.front();
      m_jobs.pop();
    }
    // execute the job
    job();
    {
      std::unique_lock<std::mutex> lock(m_completion_mutex);
      m_active_jobs--;
    }
    m_completion_condition.notify_all(); // make some noise
  }
}

void ThreadPool::queueJob(const std::function<void()>& job){
  {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    m_jobs.push(job);
    m_active_jobs++;
  }
  // wake up any locked threads
  m_queue_condition.notify_one();
}

void ThreadPool::wait(){
  std::unique_lock<std::mutex> lock(m_completion_mutex);
  m_completion_condition.wait(lock, [this]{
    return m_active_jobs.load() <= 0;
  });
}

void ThreadPool::stop(){
  {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    shouldTerminate = true;
  }
  m_queue_condition.notify_all();
  for(std::thread& activeThread : m_threads){
    activeThread.join();
  }
  m_threads.clear();
}
