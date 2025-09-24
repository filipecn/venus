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
    virtual void update() {};
    HERMES_NODISCARD virtual Projection *copy() const {
      auto *c = new Projection();
      *c = *this;
      return c;
    }

    float ratio{1.f};                 //!< film size ratio
    float near{0.01f};                //!< near depth clip plane
    float far{1000.f};                //!< far depth clip plane
    hermes::geo::vec2 clip_size;      //!< window size (in pixels)
    hermes::geo::Transform transform; //!< projection transform
    hermes::geo::Transform inv_transform;
    hermes::transform_options options{
        hermes::geo::transform_option_bits::left_handed}; //!< flags to choose
                                                          //!< handedness, etc.
  };

  /// \param p projection type
  Camera() = default;
  virtual ~Camera() = default;

  /// \return MVP transform
  HERMES_NODISCARD virtual hermes::geo::Transform transform() const;
  /// \return camera projection const ptr
  HERMES_NODISCARD const Camera::Projection *projection() const;
  /// \return projection transform
  HERMES_NODISCARD hermes::geo::Transform projectionTransform() const;
  /// \return model transform
  HERMES_NODISCARD virtual hermes::mat3 normalMatrix() const;
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
  virtual Camera &setUpVector(hermes::geo::vec3 u);
  /// \param p target position
  virtual Camera &setTargetPosition(hermes::geo::point3 p);
  /// \param p eye position
  virtual Camera &setPosition(hermes::geo::point3 p);
  /// \param z
  virtual Camera &setZoom(float z);

  /// \param w width in pixels
  /// \param h height in pixels
  virtual void resize(float w, float h);
  virtual void update();

protected:
  hermes::geo::Transform view_, inv_view_;
  hermes::geo::Transform model_, inv_model_;
  // hermes::Frustum frustum;
  hermes::mat3 normal_;
  hermes::geo::vec3 up_;
  hermes::geo::point3 pos_;
  hermes::geo::point3 target_;
  std::shared_ptr<Camera::Projection> projection_;
  float zoom_{1.};
  bool needs_update_{true};
};

class PerspectiveProjection : public Camera::Projection {
public:
  PerspectiveProjection() = default;
  /// \param fov field of view angle (in degrees)
  /// \param options handedness, zero_to_one, flip_y, and other options
  explicit PerspectiveProjection(
      float fov, hermes::transform_options options =
                     hermes::geo::transform_option_bits::left_handed);
  void update() override;
  HERMES_NODISCARD Camera::Projection *copy() const override;

  float fov{45.f}; //!< field of view angle in degrees
};

class OrthographicProjection : public Camera::Projection {
public:
  OrthographicProjection();
  OrthographicProjection(float left, float right, float bottom, float top,
                         hermes::transform_options options =
                             hermes::geo::transform_option_bits::left_handed);
  /// \param z
  void zoom(float z);
  /// \param left
  /// \param right
  /// \param bottom
  /// \param top
  void set(float left, float right, float bottom, float top);
  void update() override;
  HERMES_NODISCARD Camera::Projection *copy() const override;

private:
  hermes::geo::bounds::bbox2 region_;
};

} // namespace venus::scene
