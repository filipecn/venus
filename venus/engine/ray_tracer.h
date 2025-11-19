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

/// \file   ray_tracer.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-11-12
/// \brief  Ray tracer.

#pragma once

#include <venus/engine/graphics_engine.h>
#include <venus/mem/image.h>
#include <venus/scene/acceleration_structure.h>
#include <venus/scene/scene_graph.h>

namespace venus::engine {
class RayTracer {
public:
  VENUS_DECLARE_RAII_FUNCTIONS(RayTracer)
  void destroy() noexcept;
  void swap(RayTracer &rhs);

private:
  scene::AccelerationStructure as_;
  mem::AllocatedImage image_;
};

}; // namespace venus::engine
