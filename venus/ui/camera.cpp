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

/// \file   camera.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-09-16

#include <venus/ui/camera.h>

#include <hermes/geometry/queries.h>

namespace venus::ui {

hermes::geo::point3 hitViewPlane(scene::Camera::Ptr camera,
                                 const hermes::geo::point2 &ndc) {
  hermes::geo::Line l = camera->viewLineFromWindow(ndc);
  hermes::geo::Plane vp = camera->viewPlane(camera->targetPosition());
  hermes::geo::point3 hp;
  hermes::geo::queries::intersect(vp, l, &hp);
  return hp;
}

void noneModeBvr(scene::Camera::Ptr camera, CameraController::InputState &input,
                 const hermes::geo::point2 &p, const hermes::geo::vec2 &d) {
  HERMES_UNUSED_VARIABLE(camera);
  HERMES_UNUSED_VARIABLE(input);
  HERMES_UNUSED_VARIABLE(p);
  HERMES_UNUSED_VARIABLE(d);
}

void scaleModeBvr(scene::Camera::Ptr camera,
                  CameraController::InputState &input,
                  const hermes::geo::point2 &p, const hermes::geo::vec2 &d) {
  if (d == hermes::geo::vec2())
    return;
  HERMES_UNUSED_VARIABLE(p);
  HERMES_UNUSED_VARIABLE(input);
  float scale = (d.y < 0.f) ? 1.1f : 0.9f;
  // TODO what if the projection is different?!
  auto *proj =
      dynamic_cast<scene::OrthographicProjection *>(camera->projection());
  proj->zoom(scale);
}

void panModeBvr(scene::Camera::Ptr camera, CameraController::InputState &input,
                const hermes::geo::point2 &p, const hermes::geo::vec2 &d) {
  HERMES_UNUSED_VARIABLE(d);
  if (!input.dragging)
    return;
  hermes::geo::point3 a = hitViewPlane(camera, input.last_position);
  hermes::geo::point3 b = hitViewPlane(camera, p);
  // it is -(b - a), because moving the mouse up should move the camera down
  // (so the image goes up)
  auto translation = a - b;
  camera->setTargetPosition(camera->targetPosition() + translation);
  camera->setPosition(camera->position() + translation);
}

void zModeBvr(scene::Camera::Ptr camera, CameraController::InputState &input,
              const hermes::geo::point2 &p, const hermes::geo::vec2 &d) {
  HERMES_UNUSED_VARIABLE(d);
  if (!input.dragging)
    return;
  hermes::geo::point3 a = hitViewPlane(camera, input.start);
  hermes::geo::point3 b = hitViewPlane(camera, p);
  hermes::geo::vec3 t =
      hermes::geo::normalize(camera->targetPosition() - camera->position());
  if (p.y - input.last_position.y < 0.f)
    t *= -1.f;
  t *= hermes::geo::distance(a, b);
  camera->setTargetPosition(camera->targetPosition() + t);
  camera->setPosition(camera->position() + t);
}

void orbitModeBvr(scene::Camera::Ptr camera,
                  CameraController::InputState &input,
                  const hermes::geo::point2 &p, const hermes::geo::vec2 &d) {
  HERMES_UNUSED_VARIABLE(d);
  if (!input.dragging || p == input.last_position)
    return;
  auto direction = p - input.last_position;

  // if it is too close from the poles, we allow just horizontal rotation
  auto up_angle = hermes::geo::dot(
      (camera->position() - camera->targetPosition()).normalized(),
      camera->upVector());
  if (1.f - std::fabs(up_angle) < 1e-3 && up_angle * direction.y < 0)
    direction.y = 0;
  if (std::abs(direction.x) > std::abs(direction.y))
    direction.y = 0;
  if (std::abs(direction.x) < std::abs(direction.y))
    direction.x = 0;

  auto t = camera->position() - camera->targetPosition();
  auto u = camera->upVector();
  auto left = hermes::geo::cross(t.normalized(), u).normalized();
  auto new_pos = camera->position();

  // translate target to center
  new_pos -= (hermes::geo::vec3)camera->targetPosition();

  // the distance between coordinates -1 and 1 is equivalent to a full rotation
  // here we rotate camera position
  f32 angle = hermes::math::constants::pi * direction.x;
  auto transform = hermes::geo::Transform::rotate(angle, u);
  angle = hermes::math::constants::pi * direction.y;
  transform = transform * hermes::geo::Transform::rotate(-angle, left);
  new_pos = camera->targetPosition() + (hermes::geo::vec3)transform(new_pos);

  camera->setPosition(new_pos);
}

void zoomModeBvr(scene::Camera::Ptr camera, CameraController::InputState &input,
                 const hermes::geo::point2 &p, const hermes::geo::vec2 &d) {
  HERMES_UNUSED_VARIABLE(input);
  HERMES_UNUSED_VARIABLE(p);
  if (d == hermes::geo::vec2())
    return;
  // get distance
  auto direction = camera->targetPosition() - camera->position();
  auto distance = direction.length();
  auto s = distance * 0.1f * (d.y < 0 ? -1.f : 1.f);
  camera->setPosition(camera->position() + s * direction.normalized());
}

void firstPersonModeBvr(scene::Camera::Ptr camera,
                        CameraController::InputState &input,
                        const hermes::geo::point2 &p,
                        const hermes::geo::vec2 &d) {
  HERMES_UNUSED_VARIABLE(d);
  if (!input.dragging || p == input.last_position)
    return;
  auto direction = p - input.last_position;

  // if it is too close from the poles, we allow just horizontal rotation
  auto up_angle = hermes::geo::dot(
      (camera->position() - camera->targetPosition()).normalized(),
      camera->upVector());
  if (1.f - std::fabs(up_angle) < 1e-3 && up_angle * direction.y < 0)
    direction.y = 0;
  if (std::abs(direction.x) > std::abs(direction.y))
    direction.y = 0;
  if (std::abs(direction.x) < std::abs(direction.y))
    direction.x = 0;

  auto t = camera->position() - camera->targetPosition();
  auto u = camera->upVector();
  auto left = hermes::geo::cross(t.normalized(), u).normalized();
  auto new_target = camera->targetPosition();

  // translate target to center
  new_target -= (hermes::geo::vec3)camera->position();

  // the distance between coordinates -1 and 1 is equivalent to a full rotation
  // here we rotate camera position
  f32 angle = hermes::math::constants::pi * direction.x;
  auto transform = hermes::geo::Transform::rotate(angle, u);
  angle = hermes::math::constants::pi * direction.y;
  transform = transform * hermes::geo::Transform::rotate(-angle, left);
  new_target = camera->position() + (hermes::geo::vec3)transform(new_target);

  camera->setTargetPosition(new_target);
}

CameraController::CameraController() noexcept {
  modes_[CameraController::ControlType::NONE] = behaviors_.size();
  behaviors_.emplace_back(noneModeBvr);
  modes_[CameraController::ControlType::SCALE] = behaviors_.size();
  behaviors_.emplace_back(scaleModeBvr);
  modes_[CameraController::ControlType::PAN] = behaviors_.size();
  behaviors_.emplace_back(panModeBvr);
  modes_[CameraController::ControlType::Z] = behaviors_.size();
  behaviors_.emplace_back(zModeBvr);
  modes_[CameraController::ControlType::ORBIT] = behaviors_.size();
  behaviors_.emplace_back(orbitModeBvr);
  modes_[CameraController::ControlType::ZOOM] = behaviors_.size();
  behaviors_.emplace_back(zoomModeBvr);
  modes_[CameraController::ControlType::FIRST_PERSON] = behaviors_.size();
  behaviors_.emplace_back(firstPersonModeBvr);
}

void CameraController::mouseButton(Action action, MouseButton button,
                                   const hermes::geo::point2 &ndc) {
  if (!input_.contains(static_cast<u32>(button)))
    return;
  if (action == Action::PRESS) {
    input_state_.mode = input_[static_cast<u32>(button)];
    auto it = modes_.find(input_state_.mode);
    if (it != modes_.end())
      start(ndc);
  } else {
    auto it = modes_.find(input_state_.mode);
    if (it != modes_.end())
      stop(ndc);
    input_state_.mode = CameraController::ControlType::NONE;
  }
}

void CameraController::mouseMove(const hermes::geo::point2 &ndc) {
  if (input_state_.mode == CameraController::ControlType::NONE)
    return;
  auto it = modes_.find(input_state_.mode);
  if (it == modes_.end())
    return;
  if (behaviors_.size() > it->second && camera_) {
    behaviors_[it->second](camera_, input_state_, ndc, {});
    input_state_.last_position = ndc;
  }
}

void CameraController::mouseScroll(const hermes::geo::point2 &ndc,
                                   const hermes::geo::vec2 &d) {
  if (input_state_.mode == CameraController::ControlType::NONE)
    return;
  auto it = modes_.find(input_state_.mode);
  if (it == modes_.end())
    return;
  if (behaviors_.size() > it->second && camera_)
    behaviors_[it->second](camera_, input_state_, ndc, d);
  input_state_.mode = CameraController::ControlType::NONE;
}

void CameraController::setCamera(scene::Camera::Ptr camera) {
  camera_ = camera;
}

void CameraController::addControl(MouseButton button,
                                  CameraController::ControlType type) {
  if (static_cast<u32>(type) <
      static_cast<u32>(CameraController::ControlType::CUSTOM)) {
    input_[static_cast<u32>(button)] = type;
  } else {
    HERMES_NOT_IMPLEMENTED;
  }
}

void CameraController::clear() { input_.clear(); }

void CameraController::reset() {
  input_state_.mode = CameraController::ControlType::NONE;
}

void CameraController::start(const hermes::geo::point2 &p) {
  input_state_.start = p;
  input_state_.last_position = p;
  input_state_.dragging = true;
}

void CameraController::stop(const hermes::geo::point2 &p) {
  input_state_.last_position = p;
  input_state_.dragging = false;
}

} // namespace venus::ui
