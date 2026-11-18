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

/// \file   camera.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-09-16
/// \brief  UI Camera.

#pragma once

#include <venus/io/input.h>
#include <venus/scene/camera.h>

namespace venus::ui {

/// A camera that can be controlled by input.
class CameraController {
public:
  /// Defines the behavior of a camera been manipulated.
  enum class ControlType {
    NONE = 0,
    PAN = 1,
    ROTATE = 2,
    SCALE = 3,
    Z = 4,
    ZOOM = 5,
    ORBIT = 6,
    SCROLL_ORBIT = 7,
    FIRST_PERSON = 8,
    DOLLY = 9,
    CUSTOM = 10
  };

  struct ControlState {
    bool dragging{false};
    hermes::geo::point2f start;
    hermes::geo::point2f last_position;
    ControlType mode;
  };

  CameraController() noexcept;
  virtual ~CameraController() noexcept = default;

  /// process mouse button event
  /// \param action event type
  /// \param button button code
  /// \param p normalized mouse position (NDC)
  void mouseButton(io::Action action, io::MouseButton button,
                   const hermes::geo::point2f &ndc);
  /// process mouse move event
  /// \param p normalized mouse position (NDC)
  void mouseMove(const hermes::geo::point2f &ndc);
  /// process mouse wheel event
  /// \param p normalized mouse position (NDC)
  /// \param d scroll vector
  void mouseScroll(const hermes::geo::point2f &ndc,
                   const hermes::geo::vec2f &d);
  /// Set camera object to be manipulated by this constroller.
  CameraController &setCamera(scene::Camera::Ptr camera);
  /// Add control to camera
  /// \param button activation button for this control mode
  /// \param type control type
  CameraController &addControl(io::MouseButton button, ControlType type);
  /// Add scroll control to camera
  /// \param type control type
  CameraController &addScrollControl(ControlType type);
  /// Clear controls.
  void clear();
  /// Reset state.
  void reset();

protected:
  /// Starts the manipulation (usually triggered by a button press)
  /// \param tb trackball reference
  /// \param camera active viewport camera
  /// \param p mouse position in normalized window position (NPos)
  void start(const hermes::geo::point2f &p);
  /// Stops the manipulation (usually after a button release)
  /// \param p mouse position in normalized window position (NPos)
  void stop(const hermes::geo::point2f &p);

  scene::Camera::Ptr camera_;
  ControlState input_state_;
  std::unordered_map<h_index, ControlType> input_;
  std::unordered_map<ControlType, h_index> modes_;
  std::vector<std::function<void(scene::Camera::Ptr, ControlState &,
                                 const hermes::geo::point2f &,
                                 const hermes::geo::vec2f &)>>
      behaviors_;
};

} // namespace venus::ui
