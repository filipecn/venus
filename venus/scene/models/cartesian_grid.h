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

/// \file   cartesian_grid.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-02-17

#pragma once

#include <venus/scene/model.h>

namespace venus::scene::models {

class CartesianGrid : public Model {
public:
  using Ptr = hermes::Ref<CartesianGrid>;

  struct Config {
    Config() noexcept;
    Config &setBounds(const hermes::geo::bounds::bbox3 &bounds);
    Result<CartesianGrid> build(const engine::GraphicsDevice &gd) const;

  private:
    hermes::geo::bounds::bbox3 bounds_;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(CartesianGrid);

  void destroy() noexcept;
  void swap(CartesianGrid &rhs);

private:
  // data
  Model::Storage<mem::AllocatedBuffer> storage_;
  // material
  Material::Ptr material_;
};

} // namespace venus::scene::models