/* Copyright (c) 2026, FilipeCN.
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

/// \file   picking.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-09-04
/// \brief

#pragma once

#include <venus/mem/image.h>
#include <venus/pipeline/rasterizer.h>
#include <venus/scene/scene_graph.h>

#include <optional>
#include <unordered_map>

namespace venus::ui {

/// \brief Picking method for picking a the object rendered at a given pixel.
class Picking {
public:
  /// Result for a completed request. object_index addresses the render object
  /// ordering used by the frame that serviced the request.
  struct PickResult {
    bool hit{false};
    u32 object_index{0};
    u32 primitive_index{0};
  };

  /// Builder for picking.
  struct Config {
    /// Sets the size of the picking image.
    Config &setImageSize(const VkExtent2D &size);
    Result<Picking> build(const core::Device &device);

  private:
    VkExtent2D size_ = {1, 1};
  };

  VeResult resize(const core::Device &device, const VkExtent2D &size);
  /// Schedule one pixel for asynchronous picking. The result becomes
  /// available after the next frame has completed and collectResult runs.
  VeResult request(VkOffset2D pixel);
  /// Collect the previous frame's GPU result. The frame loop calls this only
  /// after waiting for the frame fence.
  VeResult collectResult();
  std::optional<PickResult> takeResult();
  VeResult pick(const pipeline::CommandBuffer &cb,
                const scene::RasterContext &ctx,
                const hermes::geo::Transform &view_projection);

  void destroy() noexcept;

private:
  VkExtent2D size_ = {1, 1};
  mem::AllocatedImage picking_image_;
  mem::Image::View picking_image_view_;
  mem::AllocatedImage depth_image_;
  mem::Image::View depth_image_view_;
  mem::AllocatedBuffer readback_buffer_;
  std::unordered_map<VkDeviceSize, scene::Material::Instance::Ptr>
      picking_materials_;
  VkOffset2D requested_pixel_{};
  std::optional<PickResult> result_;
  bool request_pending_{false};
  bool readback_pending_{false};
};

} // namespace venus::ui
