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

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
#include <venus/utils/vk_debug.h>
#endif

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
  VertexLayout &pushComponent(ComponentType component, VkFormat format);
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
  /// \return the offset if found, VeResult::NotFound otherwise.
  HERMES_NODISCARD Result<VkDeviceSize>
  componentOffset(ComponentType component) const;
  /// \return The stride (in bytes) of this vertex layout.
  VkDeviceSize stride() const;
  /// \return The list of vertex components.
  const std::vector<Component> &components() const;

  friend bool operator==(const VertexLayout &lhs, const VertexLayout &rhs);
  friend bool operator!=(const VertexLayout &lhs, const VertexLayout &rhs);

private:
  std::vector<Component> components_;
  VkDeviceSize stride_{0};

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
  friend struct hermes::DebugTraits<VertexLayout>;
#endif
};

bool operator==(const VertexLayout::Component &lhs,
                const VertexLayout::Component &rhs);
bool operator!=(const VertexLayout::Component &lhs,
                const VertexLayout::Component &rhs);

} // namespace venus::mem

#ifdef VENUS_INCLUDE_DEBUG_TRAITS

namespace hermes {

template <> struct DebugTraits<venus::mem::VertexLayout::ComponentType> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::mem::VertexLayout::ComponentType &data) {
#define TO_STR(C)                                                              \
  if (data == venus::mem::VertexLayout::ComponentType::C)                      \
  return DebugMessage(#C)
    TO_STR(Position);
    TO_STR(Normal);
    TO_STR(Color);
    TO_STR(UV);
    TO_STR(Tangent);
    TO_STR(Bitangent);
    TO_STR(Scalar);
    TO_STR(Vec2);
    TO_STR(Vec3);
    TO_STR(Vec4);
    TO_STR(M3x3);
    TO_STR(M4x4);
    TO_STR(Array);
#undef TO_STR
    return DebugMessage("<invalid component type>");
  }
};

template <> struct DebugTraits<venus::mem::VertexLayout> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage message(const venus::mem::VertexLayout &data) {
    return DebugMessage()
        .addTitle("Vertex Layout")
        .add("stride", data.stride_)
        .addArray<venus::mem::VertexLayout::Component>(
            "components", data.components_,
            [](h_index i, const venus::mem::VertexLayout::Component &component)
                -> DebugMessage {
              return DebugMessage(
                  "{} {} {}", VENUS_VK_STRING(VkFormat, component.format),
                  component.offset, venus::to_string(component.type));
            });
  }
};

} // namespace hermes

#endif // VENUS_INCLUDE_DEBUG_TRAITS
