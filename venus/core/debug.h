#pragma once

#include <cassert>
#include <format>
#include <iostream>
#include <string>
#include <thread>
#include <venus/core/types.h>

namespace venus::core {

struct term_colors {

  /// \brief Set of 256-terminal color codes
  /// \note Extracted from
  /// https://misc.flogisoft.com/bash/tip_colors_and_formatting

  // SET
  static const char bold[5];       //!< "\e[1m"
  static const char dim[5];        //!< "\e[2m"
  static const char underlined[5]; //!< "\e[4m"
  static const char blink[5];      //!< "\e[5m"
  static const char inverted[5];   //!< "\e[7m"
  static const char hidden[5];     //!< "\e[8m"
  // RESET
  static const char reset[5];            //!< "\e[0m"
  static const char reset_bold[6];       //!< "\e[21m"
  static const char reset_dim[6];        //!< "\e[22m"
  static const char reset_underlined[6]; //!< "\e[24m"
  static const char reset_blink[6];      //!< "\e[25m"
  static const char reset_inverted[6];   //!< "\e[27m"
  static const char reset_hidden[6];     //!< "\e[28m"
  // 8/16 Colors
  static const char default_color[6];            //!< "\e[39m"
  static const char black[6];                    //!< "\e[30m"
  static const char red[6];                      //!< "\e[31m"
  static const char green[6];                    //!< "\e[32m"
  static const char yellow[6];                   //!< "\e[33m"
  static const char blue[6];                     //!< "\e[34m"
  static const char magenta[6];                  //!< "\e[35m"
  static const char cyan[6];                     //!< "\e[36m"
  static const char light_gray[6];               //!< "\e[37m"
  static const char dark_gray[6];                //!< "\e[90m"
  static const char light_red[6];                //!< "\e[91m"
  static const char light_green[6];              //!< "\e[92m"
  static const char light_yellow[6];             //!< "\e[93m"
  static const char light_blue[6];               //!< "\e[94m"
  static const char light_magenta[6];            //!< "\e[95m"
  static const char light_cyan[6];               //!< "\e[96m"
  static const char white[6];                    //!< "\e[97m"
  static const char background_default_color[6]; //!< "\e[49m"
  static const char background_black[6];         //!< "\e[40m"
  static const char background_red[6];           //!< "\e[41m"
  static const char background_green[6];         //!< "\e[42m"
  static const char background_yellow[6];        //!< "\e[43m"
  static const char background_blue[6];          //!< "\e[44m"
  static const char background_magenta[6];       //!< "\e[45m"
  static const char background_cyan[6];          //!< "\e[46m"
  static const char background_light_gray[6];    //!< "\e[47m"
  static const char background_dark_gray[7];     //!< "\e[100m"
  static const char background_light_red[7];     //!< "\e[101m"
  static const char background_light_green[7];   //!< "\e[102m"
  static const char background_light_yellow[7];  //!< "\e[103m"
  static const char background_light_blue[7];    //!< "\e[104m"
  static const char background_light_magenta[7]; //!< "\e[105m"
  static const char background_light_cyan[7];    //!< "\e[106m"
  static const char background_white[7];         //!< "\e[107m"

  /// \brief Get 88/256 color code
  /// \param color_number
  /// \return
  inline static std::string color(u8 color_number) {
    return std::string("\e[38;5;") + std::to_string(color_number) + "m";
  }
  /// \brief Get 88/256 background color code
  /// \param color_number
  /// \return
  inline static std::string background_color(u8 color_number) {
    return std::string("\e[48;5;") + std::to_string(color_number) + "m";
  }
  /// \brief Combine two color codes
  /// \param a
  /// \param b
  /// \return
  inline static std::string combine(const std::string &a,
                                    const std::string &b) {
    return "\e[" + a.substr(2, a.size() - 3) + ";" + b.substr(2, b.size() - 3) +
           "m";
  }

  template <typename T> static std::string numberColor(T n) {
    return term_colors::color(static_cast<u8>(n));
  }

  inline static std::string threadColor(std::thread::id thread_id) {
    return term_colors::numberColor(std::hash<std::thread::id>()(thread_id));
  }

  inline static std::string random() {
    static u8 next = 0;
    next += 13;
    return color(next);
  }
};

struct Log {
  enum class Level {
    debug = 0,
    trace = 1,
    info = 2,
    warn = 3,
    error = 4,
    critical = 5,
    COUNT = 6
  };
  struct Location {
    const char *filename;
    int line;
  };

  // log config
  static std::size_t s_filename_len;
  static Log::Level s_level;
  static std::ostream *s_os;
  static u8 s_colors[static_cast<std::size_t>(Level::COUNT)];
  static bool s_use_colors;

  static void init();

  static void message(Log::Level level, Location location,
                      const std::string &s);
};

} // namespace venus::core

#ifndef VENUS_UNUSED_VARIABLE
#define VENUS_UNUSED_VARIABLE(x) ((void)x);
#endif

#ifndef VENUS_ASSERT_M
#define VENUS_ASSERT_M(exp, ...)                                               \
  {                                                                            \
    if (exp) {                                                                 \
    } else {                                                                   \
      venus::core::Log::message(                                               \
          venus::core::Log::Level::critical,                                   \
          venus::core::Log::Location{__FILE__, __LINE__},                      \
          std::format(__VA_ARGS__));                                           \
    }                                                                          \
  }
#endif

#ifndef VENUS_ASSERT
#define VENUS_ASSERT(exp)                                                      \
  {                                                                            \
    if (exp) {                                                                 \
    } else {                                                                   \
      venus::core::Log::message(                                               \
          venus::core::Log::Level::critical,                                   \
          venus::core::Log::Location{__FILE__, __LINE__}, "assert");           \
    }                                                                          \
  }
#endif

#ifndef VENUS_CHECK
#define VENUS_CHECK(exp, ...)                                                  \
  {                                                                            \
    if (exp) {                                                                 \
    } else {                                                                   \
      venus::core::Log::message(                                               \
          venus::core::Log::Level::warn,                                       \
          venus::core::Log::Location{__FILE__, __LINE__},                      \
          std::format(__VA_ARGS__));                                           \
    }                                                                          \
  }
#endif

#ifdef NDEBUG

#define ASSERT(exp)
#define VENUS_DEBUG(...)
#define VENUS_TRACE(...)
#define VENUS_INFO(...)
#define VENUS_WARN(...)
#define VENUS_ERROR(...)
#define VENUS_CRITICAL(...)

#else

#ifndef VENUS_DEBUG_VAL
#define VENUS_DEBUG_VAL(VAL)                                                   \
  venus::core::Log::message(venus::core::Log::Level::debug,                    \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            std::format("{:5} = {}", #VAL, VAL))
#endif

#ifndef VENUS_DEBUG
#define VENUS_DEBUG(...)                                                       \
  venus::core::Log::message(venus::core::Log::Level::debug,                    \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            std::format(__VA_ARGS__))
#endif

#ifndef VENUS_TRACE
#define VENUS_TRACE(...)                                                       \
  venus::core::Log::message(venus::core::Log::Level::trace,                    \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            std::format(__VA_ARGS__))
#endif

#ifndef VENUS_INFO
#define VENUS_INFO(...)                                                        \
  venus::core::Log::message(venus::core::Log::Level::info,                     \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            std::format(__VA_ARGS__))
#endif

#ifndef VENUS_WARN
#define VENUS_WARN(...)                                                        \
  venus::core::Log::message(venus::core::Log::Level::warn,                     \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            std::format(__VA_ARGS__))
#endif

#ifndef VENUS_ERROR
#define VENUS_ERROR(...)                                                       \
  venus::core::Log::message(venus::core::Log::Level::error,                    \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            std::format(__VA_ARGS__))
#endif

#ifndef VENUS_CRITICAL
#define VENUS_CRITICAL(...)                                                    \
  venus::core::Log::message(venus::core::Log::Level::critical,                 \
                            venus::core::Log::Location{__FILE__, __LINE__},    \
                            std::format(__VA_ARGS__))
#endif

#endif // NDEBUG
