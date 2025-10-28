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

#include <venus/utils/macros.h>

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(scene::OrthographicProjection)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(scene::PerspectiveProjection)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(scene::Camera::Projection)
// force update
auto t = *object;
HERMES_UNUSED_VARIABLE(t);
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_FIELD(ratio_)
HERMES_PUSH_DEBUG_FIELD(near_)
HERMES_PUSH_DEBUG_FIELD(far_)
HERMES_PUSH_DEBUG_HERMES_FIELD(clip_size_)
HERMES_PUSH_DEBUG_HERMES_FIELD(options_)
HERMES_PUSH_DEBUG_HERMES_FIELD(transform_)
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(scene::Camera)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_HERMES_FIELD(pos_)
HERMES_PUSH_DEBUG_HERMES_FIELD(target_)
HERMES_PUSH_DEBUG_HERMES_FIELD(up_)
HERMES_PUSH_DEBUG_HERMES_FIELD(view_)
HERMES_PUSH_DEBUG_LINE("{}", venus::to_string(*object.projection_))
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus

namespace venus::scene {

hermes::geo::Transform Camera::transform() const {
  if (needs_update_)
    update();
  return **projection_ * view_ * model_;
}

Camera::Projection *Camera::projection() {
  if (needs_update_)
    update();
  return projection_.get();
}

const Camera::Projection *Camera::projection() const {
  if (needs_update_)
    update();
  return projection_.get();
}

hermes::geo::Transform Camera::projectionTransform() const {
  if (needs_update_)
    update();
  return **projection_;
}

hermes::math::mat3 Camera::normalMatrix() const {
  if (needs_update_)
    update();
  return normal_;
}

hermes::geo::Transform Camera::modelTransform() const {
  if (needs_update_)
    update();
  return model_;
}

hermes::geo::Transform Camera::viewTransform() const {
  if (needs_update_)
    update();
  return view_;
}

hermes::geo::vec3 Camera::upVector() const {
  if (needs_update_)
    update();
  return up_;
};

hermes::geo::vec3 Camera::rightVector() const {
  return hermes::geo::normalize(hermes::geo::cross(up_, target_ - pos_));
}

hermes::geo::point3 Camera::position() const { return pos_; };

hermes::geo::point3 Camera::targetPosition() const { return target_; }

hermes::geo::vec3 Camera::direction() const { return target_ - pos_; }

VENUS_DEFINE_SET_FIELD_METHOD(Camera, setUpVector, const hermes::geo::vec3 &,
                              (needs_update_ = true, up_ = value))
VENUS_DEFINE_SET_FIELD_METHOD(Camera, setPosition, const hermes::geo::point3 &,
                              (needs_update_ = true, pos_ = value))
VENUS_DEFINE_SET_FIELD_METHOD(Camera, setTargetPosition,
                              const hermes::geo::point3 &,
                              (needs_update_ = true, target_ = value))
VENUS_DEFINE_SET_FIELD_METHOD(Camera, setZoom, f32,
                              (needs_update_ = true, zoom_ = value))

void Camera::resize(f32 w, f32 h) {
  auto aspect = w / h;
  projection_->setAspectRatio(aspect);
  auto clip_size = hermes::geo::vec2(zoom_, zoom_);
  if (w < h)
    clip_size.y = clip_size.x / aspect;
  else
    clip_size.x = clip_size.y * aspect;
  projection_->setClipSize(clip_size);
  needs_update_ = true;
}

void Camera::update() const {
  auto view_vector = hermes::geo::normalize(target_ - pos_);
  if (hermes::numbers::cmp::is_zero(
          hermes::geo::cross(view_vector, up_).length2()))
    up_ = {view_vector.y, view_vector.x, view_vector.z};
  view_ = hermes::geo::Transform::lookAt(
      pos_, target_, up_, hermes::geo::transform_option_bits::right_handed);
  inv_view_ = hermes::geo::inverse(view_);
  inv_model_ = hermes::geo::inverse(model_);
  normal_ = inverse((view_ * model_).upperLeftMatrix());
  needs_update_ = false;
}

hermes::geo::Line
Camera::viewLineFromWindow(const hermes::geo::point2 &p) const {
  hermes::geo::point3 O = hermes::geo::inverse(model_ * view_)(
      hermes::geo::inverse(projection_->computeTransform()) *
      hermes::geo::point3(p.x, p.y, 0.f));
  hermes::geo::point3 D = hermes::geo::inverse(model_ * view_)(
      hermes::geo::inverse(projection_->computeTransform()) *
      hermes::geo::point3(p.x, p.y, 1.f));
  return {O, D - O};
}

hermes::geo::Plane Camera::viewPlane(const hermes::geo::point3 &p) const {
  hermes::geo::vec3 n = pos_ - p;
  if (fabs(n.length()) < 1e-8)
    n = hermes::geo::vec3(0, 0, 0);
  else
    n = hermes::geo::normalize(n);
  return {hermes::geo::normal3(n),
          hermes::geo::dot(n, hermes::geo::vec3(p.x, p.y, p.z))};
}

VENUS_DEFINE_SET_FIELD_METHOD(Camera::Projection, setClipSize,
                              const hermes::geo::vec2 &,
                              (needs_update_ = true, clip_size_ = value))
VENUS_DEFINE_SET_FIELD_METHOD(Camera::Projection, setAspectRatio, f32,
                              (needs_update_ = true, ratio_ = value))
VENUS_DEFINE_SET_FIELD_METHOD(Camera::Projection, setNear, f32,
                              (needs_update_ = true, near_ = value))
VENUS_DEFINE_SET_FIELD_METHOD(Camera::Projection, setFar, f32,
                              (needs_update_ = true, far_ = value))
VENUS_DEFINE_SET_FIELD_METHOD(Camera::Projection, addOptions,
                              hermes::geo::transform_options,
                              (needs_update_ = true, options_ |= value))

const hermes::geo::Transform &Camera::Projection::inverse() const {
  if (needs_update_) {
    transform_ = computeTransform();
    inv_transform_ = hermes::geo::inverse(transform_);
    needs_update_ = false;
  }
  return inv_transform_;
}

const hermes::geo::Transform &Camera::Projection::operator*() const {
  if (needs_update_) {
    transform_ = computeTransform();
    inv_transform_ = hermes::geo::inverse(transform_);
    needs_update_ = false;
  }
  return transform_;
}

PerspectiveProjection::PerspectiveProjection(
    f32 fov, hermes::geo::transform_options options)
    : fov_in_degrees_(fov) {
  needs_update_ = true;
  this->options_ = options;
}

hermes::geo::Transform PerspectiveProjection::computeTransform() const {
  return hermes::geo::Transform::perspective(fov_in_degrees_, this->ratio_,
                                             this->near_, this->far_, options_);
}

Camera::Projection *PerspectiveProjection::copy() const {
  auto *p = new PerspectiveProjection();
  *p = *this;
  return p;
}

OrthographicProjection::OrthographicProjection() {
  region_.lower.x = region_.lower.y = this->near_ = -1.f;
  region_.upper.x = region_.upper.y = this->far_ = 1.f;
  needs_update_ = true;
}

OrthographicProjection::OrthographicProjection(
    f32 left, f32 right, f32 bottom, f32 top,
    hermes::geo::transform_options options) {
  HERMES_UNUSED_VARIABLE(options)
  set(left, right, bottom, top);
}

void OrthographicProjection::zoom(f32 z) {
  HERMES_UNUSED_VARIABLE(z);
  HERMES_NOT_IMPLEMENTED;
  // region_ = hermes::geo::Transform2::scale({z, z})(region_);
  needs_update_ = true;
}

void OrthographicProjection::set(f32 left, f32 right, f32 bottom, f32 top) {
  region_.lower.x = left;
  region_.lower.y = bottom;
  region_.upper.x = right;
  region_.upper.y = top;
  needs_update_ = true;
}

hermes::geo::Transform OrthographicProjection::computeTransform() const {
  return hermes::geo::Transform::ortho(region_.lower.x, region_.upper.x,
                                       region_.lower.y, region_.upper.y,
                                       this->near_, this->far_);
}

Camera::Projection *OrthographicProjection::copy() const {
  auto *p = new OrthographicProjection();
  *p = *this;
  return p;
}

} // namespace venus::scene
