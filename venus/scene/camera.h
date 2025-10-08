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
/// \brief  Scene Camera object.

#pragma once

#include <venus/utils/debug.h>

#include <hermes/geometry/bounds.h>
#include <hermes/geometry/transform.h>

namespace venus::scene {

class PerspectiveProjection;
class OrthographicProjection;

class Camera {
public:
  /// Build with Perspective projection
  template <class... P> static Camera perspective(P &&...params) {
    return Camera().setProjection<PerspectiveProjection>(
        std::forward<P>(params)...);
  }
  /// Build with Orthographic projection
  template <class... P> static Camera orthographic(P &&...params) {
    return Camera().setProjection<OrthographicProjection>(
        std::forward<P>(params)...);
  }

  class Projection {
  public:
    Projection() = default;
    virtual ~Projection() = default;
    // updates projection transform after changes
    virtual hermes::geo::Transform computeTransform() const {
      HERMES_WARN("Calling base Camera::Projection::computeTransform()");
      return transform_;
    };
    HERMES_NODISCARD virtual Projection *copy() const {
      auto *c = new Projection();
      *c = *this;
      return c;
    }
    Projection &setClipSize(const hermes::geo::vec2 &clip_size);
    Projection &setAspectRatio(f32 aspect_ratio);
    Projection &setNear(f32 near);
    Projection &setFar(f32 far);
    Projection &addOptions(hermes::geo::transform_options options);
    const hermes::geo::Transform &inverse() const;
    const hermes::geo::Transform &operator*() const;

  protected:
    f32 ratio_{1.f};              //!< film size ratio
    f32 near_{0.01f};             //!< near depth clip plane
    f32 far_{1000.f};             //!< far depth clip plane
    hermes::geo::vec2 clip_size_; //!< window size (in pixels)
    hermes::geo::transform_options options_{
        hermes::geo::transform_option_bits::left_handed}; //!< flags to choose
                                                          //!< handedness, etc.

    // transforms are lazily updated based on the dirty flag

    mutable bool needs_update_{true};          //!< dirty flag
    mutable hermes::geo::Transform transform_; //!< projection transform
    mutable hermes::geo::Transform inv_transform_;

    VENUS_TO_STRING_FRIEND(Projection);
  };

  /// \param p projection type
  Camera() = default;
  virtual ~Camera() = default;

  /// \return MVP transform
  HERMES_NODISCARD virtual hermes::geo::Transform transform() const;
  /// \return camera projection const ptr
  HERMES_NODISCARD Camera::Projection *projection();
  HERMES_NODISCARD const Camera::Projection *projection() const;
  /// \return projection transform
  HERMES_NODISCARD hermes::geo::Transform projectionTransform() const;
  /// \return model transform
  HERMES_NODISCARD virtual hermes::math::mat3 normalMatrix() const;
  HERMES_NODISCARD virtual hermes::geo::Transform modelTransform() const;
  HERMES_NODISCARD virtual hermes::geo::Transform viewTransform() const;
  /// \return up vector
  HERMES_NODISCARD virtual hermes::geo::vec3 upVector() const;
  HERMES_NODISCARD virtual hermes::geo::vec3 rightVector() const;
  /// \return eye position
  HERMES_NODISCARD virtual hermes::geo::point3 position() const;
  /// \return  target position
  HERMES_NODISCARD virtual hermes::geo::point3 targetPosition() const;
  HERMES_NODISCARD virtual hermes::geo::vec3 direction() const;

  template <typename T, class... P> Camera &setProjection(P &&...params) {
    projection_ = std::make_shared<T>(std::forward<P>(params)...);
    return *this;
  }
  virtual Camera &setUpVector(const hermes::geo::vec3 &u);
  /// \param p target position
  virtual Camera &setTargetPosition(const hermes::geo::point3 &p);
  /// \param p eye position
  virtual Camera &setPosition(const hermes::geo::point3 &p);
  /// \param z
  virtual Camera &setZoom(f32 z);

  /// \param w width in pixels
  /// \param h height in pixels
  virtual void resize(f32 w, f32 h);

protected:
  virtual void update() const;
  // hermes::Frustum frustum;
  mutable hermes::geo::vec3 up_{0, 1, 0};
  hermes::geo::point3 pos_;
  hermes::geo::point3 target_;
  std::shared_ptr<Camera::Projection> projection_;
  f32 zoom_{1.};

  mutable bool needs_update_{true};
  mutable hermes::math::mat3 normal_;
  mutable hermes::geo::Transform view_;
  mutable hermes::geo::Transform inv_view_;
  mutable hermes::geo::Transform model_;
  mutable hermes::geo::Transform inv_model_;

  VENUS_TO_STRING_FRIEND(Camera);
};

class PerspectiveProjection : public Camera::Projection {
public:
  PerspectiveProjection() = default;
  /// \param fov field of view angle (in degrees)
  /// \param options handedness, zero_to_one, flip_y, and other options
  explicit PerspectiveProjection(
      f32 fov_in_degrees, hermes::geo::transform_options options =
                              hermes::geo::transform_option_bits::right_handed |
                              hermes::geo::transform_option_bits::flip_y |
                              hermes::geo::transform_option_bits::zero_to_one);
  HERMES_NODISCARD hermes::geo::Transform computeTransform() const override;
  HERMES_NODISCARD Camera::Projection *copy() const override;

private:
  f32 fov_in_degrees_{45.f}; //!< field of view angle in degrees

  VENUS_TO_STRING_FRIEND(PerspectiveProjection);
};

class OrthographicProjection : public Camera::Projection {
public:
  OrthographicProjection();
  OrthographicProjection(f32 left, f32 right, f32 bottom, f32 top,
                         hermes::geo::transform_options options =
                             hermes::geo::transform_option_bits::left_handed);
  /// \param z
  void zoom(f32 z);
  /// \param left
  /// \param right
  /// \param bottom
  /// \param top
  void set(f32 left, f32 right, f32 bottom, f32 top);
  HERMES_NODISCARD hermes::geo::Transform computeTransform() const override;
  HERMES_NODISCARD Camera::Projection *copy() const override;

private:
  hermes::geo::bounds::bbox2 region_;

  VENUS_TO_STRING_FRIEND(OrthographicProjection);
};

} // namespace venus::scene
