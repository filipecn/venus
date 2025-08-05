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

/// \file   layout.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Storage data layout for describing buffers.

#pragma once

#include <venus/core/vk_api.h>

namespace venus::mem {

/// The vertex layout describes the vertex attributes consumed by a shader
/// and/or present in a vertex buffer. Any input vertex buffer in a shader
/// must have a layout compatible to the shader's input layout.
/// The Vertex Layout descriptions of a shader and the buffer doesn't need to
/// be identical, but compatible, in the sense that the shader input vertex
/// components are present in the buffer and their corresponding formats
/// match.
class VertexLayout {
public:
  /// Types of vertex layout components.
  enum class ComponentType {
    Position,
    Normal,
    Color,
    UV,
    Tangent,
    Bitangent,
    Scalar,
    Vec2,
    Vec3,
    Vec4,
    M3x3,
    M4x4,
    Array
  };

  /// Vertex layout component.
  struct Component {
    VkFormat format;
    ComponentType type;
    VkDeviceSize offset;
  };

  /// \brief Define a new component in the layout.
  /// \note Vertex components follow the same order they are pushed.
  /// \param component The component type.
  /// \param format The component data format.
  void pushComponent(ComponentType component, VkFormat format);
  /// \param other
  /// \return true if this layout contains other's components.
  bool contains(const VertexLayout &other);
  /// \param component
  /// \return the corresponding format, if the component is found.
  HERMES_NODISCARD Result<VkFormat>
  componentFormat(ComponentType component) const;
  /// Clears object (empties components)
  void clear();
  /// \brief Finds the offset of a given component type.
  /// \note This finds the first component with the given type, but vertex
  ///       layouts may have multiple components with the same type.
  /// \param component Vertex layout component type.
  /// \return the offset if found, VeResult::NOT_FOUND otherwise.
  HERMES_NODISCARD Result<VkDeviceSize>
  componentOffset(ComponentType component) const;
  /// \return The stride (in bytes) of this vertex layout.
  VkDeviceSize stride() const;
  /// \return The list of vertex components.
  const std::vector<Component> &components() const;

private:
  std::vector<Component> components_;
  VkDeviceSize stride_{0};
};

} // namespace venus::mem
