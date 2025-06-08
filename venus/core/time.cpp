/// Copyright (c) 2025, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///\file time.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2025-06-07
///
///\brief

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
