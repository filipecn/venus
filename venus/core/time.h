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
///\file time.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2025-06-07
///
///\brief

#pragma once

#include <cassert>
#include <chrono>
#include <shared_mutex>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <venus/core/types.h>

namespace venus::core {

template <class rep, std::intmax_t num, std::intmax_t den>
auto splitTime(std::chrono::duration<rep, std::ratio<num, den>> t) {
  const auto hrs = std::chrono::duration_cast<std::chrono::hours>(t);
  const auto mins = std::chrono::duration_cast<std::chrono::minutes>(t - hrs);
  const auto secs =
      std::chrono::duration_cast<std::chrono::seconds>(t - hrs - mins);
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      t - hrs - mins - secs);
  const auto us = std::chrono::duration_cast<std::chrono::microseconds>(
      t - hrs - mins - secs - ms);

  return std::make_tuple(hrs, mins, secs, ms, us);
}

template <class rep, std::intmax_t num, std::intmax_t den>
std::string timeLabel(std::chrono::duration<rep, std::ratio<num, den>> t) {
  const auto [hrs, mins, secs, ms, us] = splitTime(t);
  return std::format("{:0>2}:{:0>2}.{:0>3}.{:0>3}", mins.count(), secs.count(),
                     ms.count(), us.count());
}

template <class rep, std::intmax_t num, std::intmax_t den>
std::string
timeDurationLabel(std::chrono::duration<rep, std::ratio<num, den>> t) {
  const auto [hrs, mins, secs, ms, us] = splitTime(t);
  std::string s;
  if (mins.count() > 0)
    return std::format("{}",
                       std::chrono::duration_cast<std::chrono::minutes>(t));
  if (secs.count() > 0)
    return std::format("{}",
                       std::chrono::duration_cast<std::chrono::seconds>(t));
  if (ms.count() > 0)
    return std::format(
        "{}", std::chrono::duration_cast<std::chrono::milliseconds>(t));
  if (us.count() > 0)
    return std::format(
        "{}", std::chrono::duration_cast<std::chrono::microseconds>(t));
  return std::format("{}",
                     std::chrono::duration_cast<std::chrono::nanoseconds>(t));
}

class SystemTime {
public:
  using WallClock = std::chrono::system_clock;
  using CPUClock = std::chrono::high_resolution_clock;
  using WallSample = std::chrono::time_point<WallClock>;
  using CPUSample = std::chrono::time_point<CPUClock>;

  struct TimeSample {
    WallSample wall_time;
    CPUSample cpu_time;
  };

  ~SystemTime();

  SystemTime(const SystemTime &) = delete;
  SystemTime &operator=(const SystemTime &) = delete;

  static void init();

  static TimeSample initTime();
  static TimeSample initTime(std::thread::id thread_id);

  static inline auto wallTime() {
    std::shared_lock<std::shared_mutex> lock(s_instance.mutex_);
    auto it = s_instance.start_.find(std::this_thread::get_id());
    assert(it != s_instance.start_.end());
    return std::chrono::duration(WallClock::now() - it->second.wall_time);
  }

  static inline auto cpuTime() {
    std::shared_lock<std::shared_mutex> lock(s_instance.mutex_);
    auto it = s_instance.start_.find(std::this_thread::get_id());
    assert(it != s_instance.start_.end());
    return std::chrono::duration(CPUClock::now() - it->second.cpu_time);
  }

  static const i64 cpu_frequency;
  static const i64 wall_frequency;

private:
  SystemTime();

  static SystemTime s_instance;
  mutable std::shared_mutex mutex_;
  std::unordered_map<std::thread::id, TimeSample> start_;
};

} // namespace venus::core
