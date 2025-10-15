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

/// \file   shapes.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/engine/shapes.h>

namespace venus::scene::shapes {

Result<Model::Mesh> triangle(const hermes::geo::point3 &a,
                             const hermes::geo::point3 &b,
                             const hermes::geo::point3 &c,
                             shape_options options) {
  HERMES_UNUSED_VARIABLE(options);
  Model::Mesh mesh;
  mesh.aos.pushField<hermes::geo::point3>("position");
  auto err = mesh.aos.resize(3);
  if (err != HeError::NO_ERROR)
    return VeResult::heError(err);
  mesh.aos.valueAt<hermes::geo::point3>(0, 0) = a;
  mesh.aos.valueAt<hermes::geo::point3>(0, 1) = b;
  mesh.aos.valueAt<hermes::geo::point3>(0, 2) = c;
  // mesh.layout.pushComponent(mem::VertexLayout::ComponentType::Position,
  //                           VK_FORMAT_R32G32B32_SFLOAT);
  return Result<Model::Mesh>(std::move(mesh));
}

} // namespace venus::scene::shapes
