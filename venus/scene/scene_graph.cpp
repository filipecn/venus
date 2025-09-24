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
HERMES_PUSH_DEBUG_FIELD(index_count)
HERMES_PUSH_DEBUG_FIELD(first_index)
HERMES_PUSH_DEBUG_VK_FIELD(index_buffer)
HERMES_PUSH_DEBUG_VK_FIELD(vertex_buffer)
HERMES_PUSH_DEBUG_FIELD(vertex_buffer_address)
HERMES_TO_STRING_DEBUG_METHOD_END

} // namespace venus

namespace venus::scene {

void Node::draw(const hermes::geo::Transform &top_matrix,
                DrawContext &context) {
  for (auto &child : children_)
    child->draw(top_matrix, context);
}

std::weak_ptr<Node> Node::parent() { return parent_; }

void Node::setParent(Node::Ptr _parent) { parent_ = _parent; }

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

ModelNode::ModelNode(Model::Ptr model) : model_{model} {}

void ModelNode::draw(const hermes::geo::Transform &top_matrix,
                     DrawContext &context) {
  auto model_matrix = top_matrix * world_matrix_;
  for (const auto &shape : model_->shapes()) {
    RenderObject render_object;
    // scene
    render_object.bounds = shape.bounds;
    render_object.transform = model_matrix;
    // mesh
    render_object.first_index = shape.index_base;
    render_object.index_count = shape.index_count;
    if (model_->vertexBuffer()) {
      render_object.vertex_buffer = model_->vertexBuffer();
      render_object.vertex_buffer_address = model_->deviceAddress();
    }
    if (model_->indexBuffer())
      render_object.index_buffer = model_->indexBuffer();
    //  shading
    render_object.material = shape.material;
    context.objects.push_back(render_object);
  }
  Node::draw(top_matrix, context);
}

Model::Ptr ModelNode::model() { return model_; }

void ModelNode::setModel(Model::Ptr model) { model_ = model; }

} // namespace venus::scene
