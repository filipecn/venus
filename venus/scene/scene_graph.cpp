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

/// \file   scene_graph.cpp
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30

#include <venus/scene/scene_graph.h>

#include <venus/utils/vk_debug.h>

namespace venus {

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::RenderObject)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_HERMES_FIELD(bounds)
HERMES_PUSH_DEBUG_HERMES_FIELD(transform)
HERMES_PUSH_DEBUG_FIELD(count)
HERMES_PUSH_DEBUG_FIELD(first_index)
HERMES_PUSH_DEBUG_VK_FIELD(index_buffer)
HERMES_PUSH_DEBUG_VK_FIELD(vertex_buffer)
HERMES_PUSH_DEBUG_FIELD(vertex_buffer_address)
HERMES_PUSH_DEBUG_LINE("material instance: 0x{:x}",
                       (uintptr_t)(object.material_instance.get()))
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::graph::Node)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_LINE("{}", object.toString())
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::graph::ModelNode)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_LINE("{}", object.toString())
HERMES_TO_STRING_DEBUG_METHOD_END

HERMES_TO_STRING_DEBUG_METHOD_BEGIN(venus::scene::graph::LabeledGraph)
HERMES_PUSH_DEBUG_TITLE
HERMES_PUSH_DEBUG_LINE("{}", object.toString())
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus

namespace venus::scene {

void Renderable::setVisible(bool visible) { visible_ = visible; }

} // namespace venus::scene

namespace venus::scene::graph {

Node::~Node() noexcept { this->destroy(); }

void Node::draw(const hermes::geo::Transform &top_matrix,
                DrawContext &context) {
  if (!visible_)
    return;
  auto node_matrix = top_matrix * world_matrix_;
  for (auto &child : children_)
    child->draw(node_matrix, context);
}

void Node::destroy() noexcept {
  for (auto &child : children_)
    child->destroy();
  children_.clear();
}

Node::Ptr Node::parent() { return parent_; }

void Node::setParent(Node::Ptr _parent) { parent_ = Node::Ptr::weak(_parent); }

void Node::addChild(Node::Ptr child) { children_.push_back(child); }

void Node::setLocalTransform(const hermes::geo::Transform &transform) {
  local_matrix_ = transform;
}

const hermes::geo::Transform &Node::localTransform() const {
  return local_matrix_;
}

void Node::updateTrasform(const hermes::geo::Transform &parent_matrix) {
  world_matrix_ = parent_matrix * local_matrix_;
  for (auto &child : children_)
    child->updateTrasform(world_matrix_);
}

std::string Node::toString(u32 tab_size) const {
  hermes::cstr s;
  s.appendLine(
      hermes::cstr::format("parent: 0x{:x}", (uintptr_t)parent_.get()));
  s.appendLine(
      hermes::cstr::format("local: {}", hermes::to_string(local_matrix_)));
  s.appendLine(
      hermes::cstr::format("world: {}", hermes::to_string(world_matrix_)));
  for (const auto &child : children_) {
    s.appendLine("Child: ");
    s.appendLine(hermes::cstr::format("{}", child->toString(tab_size)));
  }
  return s.str();
}

ModelNode::ModelNode(Model::Ptr model) : model_{model} {}

void ModelNode::draw(const hermes::geo::Transform &top_matrix,
                     DrawContext &context) {
  if (!visible_)
    return;
  auto model_matrix = top_matrix * world_matrix_;
  for (const auto &shape : model_->shapes()) {
    RenderObject render_object;
    // scene
    render_object.bounds = shape.bounds;
    render_object.transform = model_matrix;
    // mesh
    render_object.first_index = shape.index_base;
    render_object.count =
        shape.index_count ? shape.index_count : shape.vertex_count;
    if (model_->vertexBuffer()) {
      render_object.vertex_buffer = model_->vertexBuffer();
      render_object.vertex_buffer_address = model_->deviceAddress();
    }
    if (model_->indexBuffer())
      render_object.index_buffer = model_->indexBuffer();
    //  shading
    render_object.material_instance = shape.material;
    context.objects.push_back(render_object);
  }
  Node::draw(model_matrix, context);
}

void ModelNode::destroy() noexcept { model_ = Model::Ptr(); }

Model::Ptr ModelNode::model() { return model_; }

void ModelNode::setModel(Model::Ptr model) { model_ = model; }

std::string ModelNode::toString(u32 tab_size) const {
  hermes::cstr s;
  s.appendLine(hermes::cstr::format("model: {}", venus::to_string(*model_)));
  s.appendLine(hermes::cstr::format("{}", Node::toString(tab_size)));
  return s.str();
}

LabeledGraph::~LabeledGraph() noexcept { destroy(); }

void LabeledGraph::destroy() noexcept {
  for (auto &node : nodes_)
    node.second->destroy();
  nodes_.clear();
}

LabeledGraph &LabeledGraph::add(const std::string &name, const Node::Ptr &node,
                                const std::string &parent) {
  nodes_.insert(std::make_pair(name, node));
  if (parent.empty())
    addChild(node);
  else {
    HERMES_ASSERT(nodes_.contains(parent));
    nodes_[parent]->addChild(node);
  }
  return *this;
}

LabeledGraph &LabeledGraph::addModel(const std::string &name,
                                     const Model::Ptr &model,
                                     const std::string &parent) {
  return add(name, ModelNode::Ptr::shared(model), parent);
}

void LabeledGraph::draw(const hermes::geo::Transform &top_matrix,
                        DrawContext &context) {
  if (!visible_)
    return;
  Node::draw(top_matrix, context);
}

std::string LabeledGraph::toString(u32 tab_size) const {
  hermes::cstr s;
  for (const auto &item : nodes_) {
    s.appendLine(hermes::cstr::format("node {} -> 0x{:x}", item.first,
                                      (uintptr_t)item.second.get()));
  }
  s.appendLine(hermes::cstr::format("{}", Node::toString(tab_size)));
  return s.str();
}

} // namespace venus::scene::graph
