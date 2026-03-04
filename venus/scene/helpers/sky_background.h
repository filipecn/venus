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

/// \file   sky_background.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-02-17

#pragma once

#include <venus/scene/model.h>

namespace venus::scene::helpers {

class SkyBackground : public Model {
public:
  using Ptr = hermes::Ref<SkyBackground>;

  struct Config {
    Result<SkyBackground> build(const engine::GraphicsDevice &gd) const;
  };

  VENUS_DECLARE_RAII_FUNCTIONS(SkyBackground);

  void destroy() noexcept;
  void swap(SkyBackground &rhs);

private:
  // material
  Material::Ptr material_;
};

} // namespace venus::scene::helpers