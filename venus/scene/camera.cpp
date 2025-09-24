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

#include <venus/scene/camera.h>

namespace venus::scene {

hermes::geo::Transform Camera::transform() const {
  return projection_->transform * view_ * model_;
}

const Camera::Projection *Camera::projection() const {
  return projection_.get();
}

hermes::geo::Transform Camera::projectionTransform() const {
  return projection_->transform;
}

hermes::mat3 Camera::normalMatrix() const { return normal_; }

hermes::geo::Transform Camera::modelTransform() const { return model_; }

hermes::geo::Transform Camera::viewTransform() const { return view_; }

hermes::geo::vec3 Camera::upVector() const { return up_; };

hermes::geo::vec3 Camera::rightVector() const {
  return hermes::geo::normalize(hermes::geo::cross(up_, target_ - pos_));
}

hermes::geo::point3 Camera::position() const { return pos_; };

hermes::geo::point3 Camera::targetPosition() const { return target_; }

hermes::geo::vec3 Camera::direction() const { return target_ - pos_; }

Camera &Camera::setUpVector(hermes::geo::vec3 u) {
  up_ = u;
  update();
  return *this;
}

Camera &Camera::setTargetPosition(hermes::geo::point3 p) {
  target_ = p;
  update();
  return *this;
}

Camera &Camera::setPosition(hermes::geo::point3 p) {
  pos_ = p;
  update();
  return *this;
}

Camera &Camera::setZoom(float z) {
  zoom_ = z;
  update();
  return *this;
}

void Camera::resize(float w, float h) {
  projection_->ratio = w / h;
  projection_->clip_size = hermes::geo::vec2(zoom_, zoom_);
  if (w < h)
    projection_->clip_size.y = projection_->clip_size.x / projection_->ratio;
  else
    projection_->clip_size.x = projection_->clip_size.y * projection_->ratio;
  projection_->update();
  update();
}

void Camera::update() {
  auto view_vector = hermes::geo::normalize(target_ - pos_);
  if (hermes::math::check::is_zero(
          hermes::geo::cross(view_vector, up_).length2()))
    up_ = {view_vector.y, view_vector.x, view_vector.z};
  view_ = hermes::geo::Transform::lookAt(pos_, target_, up_);
  inv_view_ = hermes::geo::inverse(view_);
  inv_model_ = hermes::geo::inverse(model_);
  normal_ = inverse((view_ * model_).upperLeftMatrix());
  projection_->inv_transform = inverse(projection_->transform);
}

PerspectiveProjection::PerspectiveProjection(float fov,
                                             hermes::transform_options options)
    : fov(fov) {
  this->options = options;
}

void PerspectiveProjection::update() {
  this->transform = hermes::geo::Transform::perspective(
      fov, this->ratio, this->near, this->far, options);
}

Camera::Projection *PerspectiveProjection::copy() const {
  auto *p = new PerspectiveProjection();
  *p = *this;
  return p;
}

OrthographicProjection::OrthographicProjection() {
  region_.lower.x = region_.lower.y = this->near = -1.f;
  region_.upper.x = region_.upper.y = this->far = 1.f;
}

OrthographicProjection::OrthographicProjection(
    float left, float right, float bottom, float top,
    hermes::transform_options options) {
  HERMES_UNUSED_VARIABLE(options)
  set(left, right, bottom, top);
}

void OrthographicProjection::zoom(float z) {
  // region_ = hermes::geo::Transform2::scale({z, z})(region_);
  update();
}

void OrthographicProjection::set(float left, float right, float bottom,
                                 float top) {
  region_.lower.x = left;
  region_.lower.y = bottom;
  region_.upper.x = right;
  region_.upper.y = top;
  update();
}

void OrthographicProjection::update() {
  this->transform = hermes::geo::Transform::ortho(
      region_.lower.x, region_.upper.x, region_.lower.y, region_.upper.y,
      this->near, this->far);
}

Camera::Projection *OrthographicProjection::copy() const {
  auto *p = new OrthographicProjection();
  *p = *this;
  return p;
}

} // namespace venus::scene
