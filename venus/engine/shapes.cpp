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
  mesh.indices = {0, 1, 2};
  mesh.vertex_layout.pushComponent(mem::VertexLayout::ComponentType::Position,
                                   VK_FORMAT_R32G32B32_SFLOAT);
  return Result<Model::Mesh>(std::move(mesh));
}

Result<Model::Mesh> box(const hermes::geo::bounds::bbox3 &box,
                        shape_options options) {
  // check options
  if (options.contain(shape_option_bits::tangent_space))
    options =
        options | shape_option_bits::tangent | shape_option_bits::bitangent;
  const bool generate_wireframe = options.contain(shape_option_bits::wireframe);
  const bool only_vertices = options.contain(shape_option_bits::vertices);
  const bool generate_normals = options.contain(shape_option_bits::normal);
  const bool generate_tangents = options.contain(shape_option_bits::tangent);
  const bool generate_bitangents =
      options.contain(shape_option_bits::bitangent);
  const bool flip_normals = options.contain(shape_option_bits::flip_normals);
  const bool flip_faces = options.contain(shape_option_bits::flip_faces);
  const bool unique_positions =
      options.contain(shape_option_bits::unique_positions);
  const bool generate_uvw = options.contain(shape_option_bits::uvw);
  const bool generate_uvs = options.contain(shape_option_bits::uv);
  HERMES_UNUSED_VARIABLE(flip_faces)

  // model data
  Model::Mesh mesh;

  // data fields
  const u64 position_id = mesh.aos.pushField<hermes::geo::point3>("position");
  const u64 normal_id =
      generate_normals ? mesh.aos.pushField<hermes::geo::vec3>("normal") : 0;
  const u64 uv_id =
      generate_uvs ? mesh.aos.pushField<hermes::geo::point2>("uvs") : 0;
  const u64 uvw_id =
      generate_uvw ? mesh.aos.pushField<hermes::geo::point3>("uvw") : 0;
  const u64 tangent_id =
      generate_tangents ? mesh.aos.pushField<hermes::geo::vec3>("tangent") : 0;
  const u64 bitangent_id =
      generate_bitangents ? mesh.aos.pushField<hermes::geo::vec3>("bitangent")
                          : 0;

  HERMES_UNUSED_VARIABLE(tangent_id)
  HERMES_UNUSED_VARIABLE(bitangent_id)
  // base vertices
  hermes::geo::point3 base_vertices[8] = {
      {box.lower.x, box.lower.y, box.lower.z},  // 0
      {box.upper.x, box.lower.y, box.lower.z},  // 1
      {box.upper.x, box.upper.y, box.lower.z},  // 2
      {box.lower.x, box.upper.y, box.lower.z},  // 3
      {box.lower.x, box.lower.y, box.upper.z},  // 4
      {box.upper.x, box.lower.y, box.upper.z},  // 5
      {box.upper.x, box.upper.y, box.upper.z},  // 6
      {box.lower.x, box.upper.y, box.upper.z}}; // 7

  // base uvs
  hermes::geo::point2 base_uvs[4] = {
      {0, 0}, // 0
      {1, 0}, // 1
      {1, 1}, // 2
      {0, 1}, // 3
  };

  hermes::geo::point3 base_uvw[8] = {
      {0, 0, 0}, // 0
      {1, 0, 0}, // 1
      {1, 1, 0}, // 2
      {0, 1, 0}, // 3
      {0, 0, 1}, // 4
      {1, 0, 1}, // 5
      {1, 1, 1}, // 6
      {0, 1, 1}, // 7
  };

  hermes::geo::vec3 base_normals[6] = {
      {0, 0, -1}, // -Z
      {0, 0, 1},  // +Z
      {0, -1, 0}, // -Y
      {0, 1, 0},  // Y
      {-1, 0, 0}, // -X
      {1, 0, 0},  // X
  };

  //           7
  //  3                     6
  //               2
  //
  //           4
  //  0                     5
  //               1

  std::vector<i32> base_vertex_indices = {
      0, 1, 2, 3, // -Z face
      4, 7, 6, 5, // +Z face
      0, 4, 5, 1, // -Y face
      3, 2, 6, 7, // +Y face
      0, 3, 7, 4, // -X face
      2, 1, 5, 6  // +X face
  };

  // data type
  mesh.primitive_type = Model::Mesh::PrimitiveType::TRIANGLES;
  if (only_vertices)
    mesh.primitive_type = Model::Mesh::PrimitiveType::POINTS;
  else if (generate_wireframe)
    mesh.primitive_type = Model::Mesh::PrimitiveType::LINES;

  // compute number of vertices
  size_t n_vertices = 8;
  if (unique_positions && !only_vertices) {
    n_vertices = 24;
    if (generate_wireframe)
      n_vertices = 8 * 6;
  }
  VENUS_HE_RETURN_BAD_RESULT(mesh.aos.resize(n_vertices));

  size_t n_indices = 36;
  if (generate_wireframe)
    n_indices = 8; // a line per edge

  HERMES_UNUSED_VARIABLE(n_indices)

  // fill vertex data
  if (generate_wireframe) {
    mesh.indices = {0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 1, 5,
                    2, 6, 3, 7, 4, 5, 5, 6, 6, 7, 7, 4};
    if (unique_positions) {
    } else {
      for (u32 i = 0; i < n_vertices; ++i)
        mesh.aos.valueAt<hermes::geo::point3>(position_id, i) =
            base_vertices[i];
    }
  } else {
    // tesselate cube
    for (int f = 0; f < 6; ++f)
      for (int jump = 0; jump < 2; ++jump) {
        mesh.indices.emplace_back(base_vertex_indices[f * 4 + 0]);
        mesh.indices.emplace_back(base_vertex_indices[f * 4 + jump + 1]);
        mesh.indices.emplace_back(base_vertex_indices[f * 4 + jump + 2]);
      }

    if (unique_positions) {
      // we must duplicate each vertex per index
      // TODO
      HERMES_NOT_IMPLEMENTED;
    } else {
      HERMES_ASSERT(n_vertices == 8)
      for (u32 i = 0; i < n_vertices; ++i) {
        mesh.aos.valueAt<hermes::geo::point3>(position_id, i) =
            base_vertices[i];
        if (generate_uvw)
          mesh.aos.valueAt<hermes::geo::point3>(uvw_id, i) = base_uvw[i];
        if (generate_uvs)
          mesh.aos.valueAt<hermes::geo::point2>(uv_id, i) = base_uvs[i % 4];
      }
      if (generate_normals) {
        for (int f = 0; f < 6; ++f)
          for (int i = 0; i < 4; ++i)
            mesh.aos.valueAt<hermes::geo::vec3>(normal_id, f * 4 + i) =
                flip_normals ? -base_normals[f] : base_normals[f];
      }
    }
  }
  return Result<Model::Mesh>(std::move(mesh));
}

} // namespace venus::scene::shapes
