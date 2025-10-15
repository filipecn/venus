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

/// \file   scene.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-06-07
/// \brief  App scene.

#pragma once

#include <venus/scene/camera.h>
#include <venus/scene/scene_graph.h>

namespace venus::app {

/// Holds all resources of a scene.
class Scene {
public:
  VENUS_DECLARE_RAII_FUNCTIONS(Scene)

  void destroy() noexcept;
  void swap(Scene &rhs);

  scene::graph::LabeledGraph &graph();
  const scene::graph::LabeledGraph &graph() const;

  Scene &addMaterial(const std::string &name,
                     const scene::Material::Ptr &material);
  template <typename MaterialType, class... Args>
  HERMES_NODISCARD VeResult addNewMaterial(const std::string &name,
                                           Args &&...args) {
    scene::Material m;
    VENUS_ASSIGN_RESULT_OR_RETURN_BAD_RESULT(
        m, MaterialType::material(std::forward<Args>(args)...));
    addMaterial(name, scene::Material::Ptr::shared(std::move(m)));
    return VeResult::noError();
  }

  const scene::Material *getMaterial(const std::string &name) const;

private:
  VENUS_to_string_FRIEND(Scene);

  scene::graph::LabeledGraph graph_;
  std::unordered_map<std::string, scene::Material::Ptr> materials_;
};

} // namespace venus::app
