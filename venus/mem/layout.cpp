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

/// \file   layout.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/mem/layout.h>

namespace venus::mem {

void VertexLayout::pushComponent(VertexLayout::ComponentType component,
                                 VkFormat format) {
  components_.push_back(
      {.format = format, .type = component, .offset = stride_});
  stride_ += core::vk::formatSize(format);
}

bool VertexLayout::contains(const VertexLayout &other) {
  for (const auto &c : other.components_)
    if (!componentFormat(c.type))
      return false;
  return true;
}

Result<VkFormat>
VertexLayout::componentFormat(VertexLayout::ComponentType component) const {
  for (const auto &c : components_) {
    if (c.type == component)
      return Result<VkFormat>(c.format);
  }
  return VeResult::notFound();
}

void VertexLayout::clear() {
  components_.clear();
  stride_ = 0;
}

Result<VkDeviceSize>
VertexLayout::componentOffset(VertexLayout::ComponentType component) const {
  for (const auto &c : components_) {
    if (c.type == component)
      return Result<VkDeviceSize>(c.offset);
  }
  return VeResult::notFound();
}

VkDeviceSize VertexLayout::stride() const { return stride_; }

const std::vector<VertexLayout::Component> &VertexLayout::components() const {
  return components_;
}

} // namespace venus::mem
