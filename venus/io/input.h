/* Copyright (c) 2025, FilipeCN.
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/// \file   input.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-09-16
/// \brief  Input enumerations.

#pragma once

#include <venus/utils/debug.h>

#include <hermes/base/flags.h>
#include <hermes/geometry/point.h>

#include <variant>

namespace venus::io {
enum class modifier_bits : u32 { none = 0x00, shift = 0x01, ctrl = 0x02 };

using modifiers = hermes::Flags<modifier_bits>;
} // namespace venus::io

namespace hermes {
template <> struct FlagTraits<venus::io::modifier_bits> {
  static HERMES_CONST_OR_CONSTEXPR bool is_bitmask = true;
  static HERMES_CONST_OR_CONSTEXPR venus::io::modifiers all_flags =
      venus::io::modifier_bits::none | venus::io::modifier_bits::shift |
      venus::io::modifier_bits::ctrl;
};
} // namespace hermes

namespace venus::io {

enum class Action { RELEASE = 0, PRESS = 1, REPEAT = 2, SCROLL = 3 };

enum class MouseButton { LEFT = 10, RIGHT = 11, MIDDLE = 12 };

enum class Key {
  NUM_0 = 48,
  NUM_1 = 49,
  NUM_2 = 50,
  NUM_3 = 51,
  NUM_4 = 52,
  NUM_5 = 53,
  NUM_6 = 54,
  NUM_7 = 55,
  NUM_8 = 56,
  NUM_9 = 57,
  A = 65,
  B = 66,
  C = 67,
  D = 68,
  E = 69,
  F = 70,
  G = 71,
  H = 72,
  I = 73,
  J = 74,
  K = 75,
  L = 76,
  M = 77,
  N = 78,
  O = 79,
  P = 80,
  Q = 81,
  R = 82,
  S = 83,
  T = 84,
  U = 85,
  V = 86,
  W = 87,
  X = 88,
  Y = 89,
  Z = 90,
  ESCAPE = 256,
  ENTER = 257,
  TAB = 258,
  BACKSPACE = 259,
};

inline bool isMouseButton(u32 code) {
  return code >= static_cast<u32>(MouseButton::LEFT) &&
         code <= static_cast<u32>(MouseButton::MIDDLE);
}

inline bool isKey(u32 code) {
  return code >= static_cast<u32>(Key::NUM_0) &&
         code <= static_cast<u32>(Key::Z);
}

struct KeyEvent {
  Action action{Action::RELEASE};
  Key key{0};
  modifiers mods{modifier_bits::none};
};

struct MouseMoveEvent {
  hermes::geo::point2f pos;
};

struct MouseScrollEvent {
  hermes::geo::vec2f offset;
};

struct MouseButtonEvent {
  Action action{Action::RELEASE};
  MouseButton button{MouseButton::LEFT};
  io::modifiers mods{modifier_bits::none};
};

struct InputEvent {
  static InputEvent mouseMoveEvent(const hermes::geo::point2f &pos) {
    return InputEvent(MouseMoveEvent{.pos = pos});
  }
  static InputEvent mouseScrollEvent(const hermes::geo::vec2f &offset) {
    return InputEvent(MouseScrollEvent{.offset = offset});
  }
  static InputEvent mouseButtonEvent(Action action, MouseButton button,
                                     modifiers mods) {
    return InputEvent(
        MouseButtonEvent{.action = action, .button = button, .mods = mods});
  }
  static InputEvent keyEvent(Action action, Key key, modifiers mods) {
    return InputEvent(KeyEvent{.action = action, .key = key, .mods = mods});
  }

  InputEvent(KeyEvent &&event) : event_(std::move(event)) {}
  InputEvent(MouseMoveEvent &&event) : event_(std::move(event)) {}
  InputEvent(MouseScrollEvent &&event) : event_(std::move(event)) {}
  InputEvent(MouseButtonEvent &&event) : event_(std::move(event)) {}

  bool isKeyEvent() const { return std::holds_alternative<KeyEvent>(event_); }
  bool isMouseMoveEvent() const {
    return std::holds_alternative<MouseMoveEvent>(event_);
  }
  bool isMouseScrollEvent() const {
    return std::holds_alternative<MouseScrollEvent>(event_);
  }
  bool isMouseButtonEvent() const {
    return std::holds_alternative<MouseButtonEvent>(event_);
  }
  MouseMoveEvent mouseMove() const { return std::get<MouseMoveEvent>(event_); }
  MouseScrollEvent mouseScroll() const {
    return std::get<MouseScrollEvent>(event_);
  }
  MouseButtonEvent mouseButton() const {
    return std::get<MouseButtonEvent>(event_);
  }
  KeyEvent keyEvent() const { return std::get<KeyEvent>(event_); }

private:
  std::variant<KeyEvent, MouseMoveEvent, MouseScrollEvent, MouseButtonEvent>
      event_;
};

struct InputState {};

} // namespace venus::io
