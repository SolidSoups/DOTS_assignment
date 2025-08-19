#pragma once
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>

class ThreadPool{
public:
  ThreadPool();
  ~ThreadPool();
  void queueJob(const std::function<void()>& job);
  void stop();
  void wait();

  const uint32_t num_threads;
private:
  void threadLoop();

  std::vector<std::thread> m_threads;
  std::queue<std::function<void()>> m_jobs;

  bool shouldTerminate = false;

  std::mutex m_queue_mutex;
  std::condition_variable m_queue_condition;

  std::atomic<size_t> m_active_jobs = 0;
  std::mutex m_completion_mutex;
  std::condition_variable m_completion_condition;
};
