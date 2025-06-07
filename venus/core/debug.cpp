#include <algorithm>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <venus/core/debug.h>
#include <venus/core/time.h>

namespace venus::core {

const char term_colors::bold[5] = "\e[1m";
const char term_colors::dim[5] = "\e[2m";
const char term_colors::underlined[5] = "\e[4m";
const char term_colors::blink[5] = "\e[5m";
const char term_colors::inverted[5] = "\e[7m";
const char term_colors::hidden[5] = "\e[8m";

const char term_colors::reset[5] = "\e[0m";
const char term_colors::reset_bold[6] = "\e[21m";
const char term_colors::reset_dim[6] = "\e[22m";
const char term_colors::reset_underlined[6] = "\e[24m";
const char term_colors::reset_blink[6] = "\e[25m";
const char term_colors::reset_inverted[6] = "\e[27m";
const char term_colors::reset_hidden[6] = "\e[28m";

const char term_colors::default_color[6] = "\e[39m";
const char term_colors::black[6] = "\e[30m";
const char term_colors::red[6] = "\e[31m";
const char term_colors::green[6] = "\e[32m";
const char term_colors::yellow[6] = "\e[33m";
const char term_colors::blue[6] = "\e[34m";
const char term_colors::magenta[6] = "\e[35m";
const char term_colors::cyan[6] = "\e[36m";
const char term_colors::light_gray[6] = "\e[37m";
const char term_colors::dark_gray[6] = "\e[90m";
const char term_colors::light_red[6] = "\e[91m";
const char term_colors::light_green[6] = "\e[92m";
const char term_colors::light_yellow[6] = "\e[93m";
const char term_colors::light_blue[6] = "\e[94m";
const char term_colors::light_magenta[6] = "\e[95m";
const char term_colors::light_cyan[6] = "\e[96m";
const char term_colors::white[6] = "\e[97m";

const char term_colors::background_default_color[6] = "\e[49m";
const char term_colors::background_black[6] = "\e[40m";
const char term_colors::background_red[6] = "\e[41m";
const char term_colors::background_green[6] = "\e[42m";
const char term_colors::background_yellow[6] = "\e[43m";
const char term_colors::background_blue[6] = "\e[44m";
const char term_colors::background_magenta[6] = "\e[45m";
const char term_colors::background_cyan[6] = "\e[46m";
const char term_colors::background_light_gray[6] = "\e[47m";
const char term_colors::background_dark_gray[7] = "\e[100m";
const char term_colors::background_light_red[7] = "\e[101m";
const char term_colors::background_light_green[7] = "\e[102m";
const char term_colors::background_light_yellow[7] = "\e[103m";
const char term_colors::background_light_blue[7] = "\e[104m";
const char term_colors::background_light_magenta[7] = "\e[105m";
const char term_colors::background_light_cyan[7] = "\e[106m";
const char term_colors::background_white[7] = "\e[107m";

std::size_t Log::s_filename_len = 15;
Log::Level Log::s_level = Log::Level::debug;
std::ostream *Log::s_os{&std::cout};
u8 Log::s_colors[static_cast<std::size_t>(Log::Level::COUNT)] = {
    247, // debug
    247, // trace
    247, // info
    191, // warn
    9,   // error
    197  // critical
};
bool Log::s_use_colors = true;

void Log::init() { s_os = &std::cout; }

void Log::message(Log::Level level, Location location, const std::string &s) {
  if (level < s_level)
    return;

  static std::mutex m;
  std::unique_lock lock(m);

  static const char *level_names[6] = {"DEBUG", "TRACE", "INFO",
                                       "WARN",  "ERROR", "CRITICAL"};
  if (s_use_colors)
    *s_os << term_colors::threadColor(std::this_thread::get_id());

  *s_os << std::format("[{} | {}] ", std::this_thread::get_id(),
                       timeLabel(SystemTime::wallTime()));
  if (s_use_colors)
    *s_os << term_colors::color(s_colors[static_cast<std::size_t>(level)]);

  *s_os << std::format(
      "[{}][{:.15s}][{}] {}", level_names[static_cast<int>(level)],
      &location.filename[std::max(
          0, static_cast<int>(std::strlen(location.filename)) - 13)],
      location.line, s);

  if (s_use_colors)
    *s_os << term_colors::default_color << term_colors::reset;

  *s_os << std::endl;
}

} // namespace venus::core
