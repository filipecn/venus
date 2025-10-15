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

#include <venus/app/scene.h>

namespace venus {
HERMES_TO_STRING_DEBUG_METHOD_BEGIN(app::Scene)
HERMES_PUSH_DEBUG_TITLE
HERMES_TO_STRING_DEBUG_METHOD_END
} // namespace venus

namespace venus::app {

Scene::~Scene() noexcept {}

Scene::Scene(Scene &&rhs) noexcept { *this = std::move(rhs); }

Scene &Scene::operator=(Scene &&rhs) noexcept {
  destroy();
  swap(rhs);
  return *this;
}

void Scene::destroy() noexcept {
  graph_.destroy();
  materials_.clear();
}

void Scene::swap(Scene &rhs) { VENUS_SWAP_FIELD_WITH_RHS(graph_); }

scene::graph::LabeledGraph &Scene::graph() { return graph_; }

const scene::graph::LabeledGraph &Scene::graph() const { return graph_; }

Scene &Scene::addMaterial(const std::string &name,
                          const scene::Material::Ptr &material) {
  materials_[name] = material;
  return *this;
}

const scene::Material *Scene::getMaterial(const std::string &name) const {
  auto it = materials_.find(name);
  if (it == materials_.end())
    return nullptr;
  return it->second.get();
}

} // namespace venus::app
