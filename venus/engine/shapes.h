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
/// \brief  Venus built-in shapes.

#pragma once

#include <venus/scene/model.h>

#include <hermes/base/flags.h>
#include <hermes/storage/memory.h>

namespace venus::scene {

/// Shape's mesh attributes and configurations
enum class shape_option_bits : u32 {
  none = 0x00,   //!< the mesh contains only positions
  normal = 0x01, //!< generate vertex normals
  uv = 0x02,     //!< generate vertex uv coordinates
  uvw = 0x04,    //!< generate vertex uvw coordinates
  tangent_space =
      0x08, //!< generate vertex tangent space (stores tangents and bitangents)
  tangent = 0x10,   //!< generate vertex tangent space (stores tangents)
  bitangent = 0x20, //!< generate vertex tangent space (stores bitangents)
  unique_positions = 0x40, //!< vertex attributes are averaged to occupy a
                           //!< single index in the mesh
  wireframe = 0x80,        //!< only edges
  vertices = 0x100,        //!< only vertices
  flip_normals = 0x200, //!< flip normals to point inwards (uv coordinates may
                        //!< change as well)
  flip_faces = 0x400,   //!< reverse face vertex order
  merge = 0x800         //!< merge shape elements
};

using shape_options = hermes::Flags<shape_option_bits>;
} // namespace venus::scene

namespace hermes {
template <> struct FlagTraits<venus::scene::shape_option_bits> {
  static HERMES_CONST_OR_CONSTEXPR bool is_bitmask = true;
  static HERMES_CONST_OR_CONSTEXPR venus::scene::shape_options all_flags =
      venus::scene::shape_option_bits::none |
      venus::scene::shape_option_bits::normal |
      venus::scene::shape_option_bits::uv |
      venus::scene::shape_option_bits::uvw |
      venus::scene::shape_option_bits::tangent_space |
      venus::scene::shape_option_bits::tangent |
      venus::scene::shape_option_bits::bitangent |
      venus::scene::shape_option_bits::unique_positions |
      venus::scene::shape_option_bits::wireframe |
      venus::scene::shape_option_bits::vertices |
      venus::scene::shape_option_bits::flip_normals |
      venus::scene::shape_option_bits::flip_faces |
      venus::scene::shape_option_bits::merge;
};

} // namespace hermes

namespace venus::scene::shapes {

Result<Model::Mesh> triangle(const hermes::geo::point3 &a,
                             const hermes::geo::point3 &b,
                             const hermes::geo::point3 &c,
                             shape_options options = shape_option_bits::none);

} // namespace venus::scene::shapes
