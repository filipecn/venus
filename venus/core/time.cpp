#include <chrono>
#include <mutex>
#include <thread>
#include <venus/core/time.h>

namespace venus::core {

SystemTime SystemTime::s_instance;

const i64 SystemTime::cpu_frequency =
    SystemTime::CPUClock::period::den / SystemTime::CPUClock::period::num;

const i64 SystemTime::wall_frequency =
    SystemTime::WallClock::period::den / SystemTime::WallClock::period::num;

SystemTime::SystemTime() = default;

SystemTime::~SystemTime() = default;

void SystemTime::init() {
  std::unique_lock<std::shared_mutex> lock(s_instance.mutex_);
  if (s_instance.start_.count(std::this_thread::get_id()) == 0)
    s_instance.start_[std::this_thread::get_id()] = {
        .wall_time = WallClock::now(), .cpu_time = CPUClock::now()};
}

SystemTime::TimeSample SystemTime::initTime() {
  std::shared_lock<std::shared_mutex> lock(s_instance.mutex_);
  auto it = s_instance.start_.find(std::this_thread::get_id());
  assert(it != s_instance.start_.end());
  return it->second;
}

SystemTime::TimeSample SystemTime::initTime(std::thread::id thread_id) {
  std::shared_lock<std::shared_mutex> lock(s_instance.mutex_);
  auto it = s_instance.start_.find(thread_id);
  assert(it != s_instance.start_.end());
  return it->second;
}

} // namespace venus::core
