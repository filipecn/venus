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

/// \file   scene_graph.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2025-07-30
/// \brief  Scene graph.

#pragma once

#include <venus/scene/material.h>
#include <venus/scene/model.h>

#include <hermes/geometry/bounds.h>
#include <hermes/geometry/transform.h>

namespace venus::scene {

// *****************************************************************************
//                                                               Render Object
// *****************************************************************************

/// Holds the data and information necessary to draw a shape.
struct RenderObject {
  // scene

  hermes::geo::bounds::bsphere3 bounds;
  hermes::geo::Transform transform;

  // mesh

  u32 index_count{0};
  u32 first_index{0};
  VkBuffer index_buffer{VK_NULL_HANDLE};
  VkBuffer vertex_buffer{VK_NULL_HANDLE};
  VkDeviceAddress vertex_buffer_address{0};

  // shading

  Material::Instance::Ptr material;

  VENUS_TO_STRING_FRIEND(RenderObject);
};

// *****************************************************************************
//                                                               Draw Context
// *****************************************************************************

/// Auxiliary struct that gathers objects to be drawn during render time.
struct DrawContext {
  std::vector<RenderObject> objects;
};

// *****************************************************************************
//                                                       Renderable Interface
// *****************************************************************************

/// Interface for objects that can produce render objects.
class Renderable {
  virtual void draw(const hermes::geo::Transform &top_matrix,
                    DrawContext &context) = 0;
};

class Destroyable {
  virtual void destroy() noexcept = 0;
};

namespace graph {

// *****************************************************************************
//                                                           Scene Graph Node
// *****************************************************************************

/// Scene graph node.
class Node : public Renderable, public Destroyable {
public:
  using Ptr = hermes::Ref<Node>;

  Node() noexcept = default;
  virtual ~Node() noexcept;

  // Interface

  virtual void draw(const hermes::geo::Transform &top_matrix,
                    DrawContext &context) override;
  virtual void destroy() noexcept override;

  // access

  /// \return Node local transform matrix.
  const hermes::geo::Transform &localTransform() const;
  /// \return This node parent.
  Node::Ptr parent();

  // graph construction

  /// Sets a parent node for this node.
  /// \param parent Parent node.
  void setParent(Node::Ptr parent);
  /// Appends child to this node children list.
  /// \param child Child node.
  void addChild(Node::Ptr child);
  /// Sets a local transform for this node.
  /// \param transform Local transform matrix.
  void setLocalTransform(const hermes::geo::Transform &transform);
  /// Propagates the world transform of all nodes downwards.
  /// \param parent_matrix New transform coming from above.
  void updateTrasform(const hermes::geo::Transform &parent_matrix);

protected:
  // graph
  Node::Ptr parent_;
  std::vector<Ptr> children_;
  // geometry
  hermes::geo::Transform local_matrix_;
  hermes::geo::Transform world_matrix_;

  VENUS_TO_STRING_FRIEND(Node);
};

// *****************************************************************************
//                                                                  Model Node
// *****************************************************************************

/// Specialized scene graph node containing a model that can be rendered.
class ModelNode : public Node {
public:
  ModelNode() = default;
  virtual ~ModelNode() noexcept = default;
  ModelNode(Model::Ptr model);

  void draw(const hermes::geo::Transform &top_matrix,
            DrawContext &context) override;

  Model::Ptr model();
  void setModel(Model::Ptr model);

protected:
  Model::Ptr model_;
};

// *****************************************************************************
//                                                                 Scene Graph
// *****************************************************************************

/// Contains the graph of the scene made of scene Node.
class LabeledGraph : public Node {
public:
  LabeledGraph() noexcept = default;
  virtual ~LabeledGraph() noexcept;

  LabeledGraph &add(const std::string &name, const Node::Ptr &node,
                    const std::string &parent = "");

  void draw(const hermes::geo::Transform &top_matrix,
            DrawContext &context) override;
  void destroy() noexcept override;

private:
  std::unordered_map<std::string, Node::Ptr> nodes_;

  VENUS_TO_STRING_FRIEND(LabeledGraph);
};

} // namespace graph

} // namespace venus::scene
