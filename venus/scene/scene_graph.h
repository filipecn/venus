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

#include <venus/scene/camera.h>
#include <venus/scene/material.h>
#include <venus/scene/model.h>

#include <hermes/geometry/bounds.h>
#include <hermes/geometry/transform.h>

#ifdef VENUS_INCLUDE_GLTF
#include <venus/scene/texture.h>
#endif // VENUS_INCLUDE_GLTF

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

  u32 count{0}; //< index count or vertex count
  u32 first_index{0};
  VkBuffer index_buffer{VK_NULL_HANDLE};
  VkBuffer vertex_buffer{VK_NULL_HANDLE};
  VkDeviceAddress vertex_buffer_address{0};

  // shading

  Material::Instance::Ptr material_instance;

  VENUS_to_string_FRIEND(RenderObject);
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
public:
  virtual void draw(const hermes::geo::Transform &top_matrix,
                    DrawContext &context) = 0;

  void setVisible(bool visible);

protected:
  bool visible_{true};
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

  VENUS_to_string_FRIEND(Node);
  VENUS_VIRTUAL_toString_METHOD
};

// *****************************************************************************
//                                                                  Model Node
// *****************************************************************************

/// Specialized scene graph node containing a model that can be rendered.
class ModelNode : public Node {
public:
  using Ptr = hermes::Ref<ModelNode>;

  ModelNode() = default;
  virtual ~ModelNode() noexcept = default;
  ModelNode(Model::Ptr model);

  void draw(const hermes::geo::Transform &top_matrix,
            DrawContext &context) override;
  void destroy() noexcept override;

  Model::Ptr model();
  void setModel(Model::Ptr model);

protected:
  Model::Ptr model_;

  VENUS_to_string_FRIEND(ModelNode);
  VENUS_VIRTUAL_toString_METHOD_OVERRIDE
};

// *****************************************************************************
//                                                                 Camera Node
// *****************************************************************************

/// Specialized scene graph node containing a camera.
class CameraNode : public Node {
public:
  using Ptr = hermes::Ref<CameraNode>;

  CameraNode() = default;
  virtual ~CameraNode() noexcept = default;
  CameraNode(Camera::Ptr camera);

  void draw(const hermes::geo::Transform &top_matrix,
            DrawContext &context) override;
  void destroy() noexcept override;

  Camera::Ptr camera();
  void setCamera(Camera::Ptr camera);

protected:
  Camera::Ptr camera_;

  VENUS_to_string_FRIEND(CameraNode);
  VENUS_VIRTUAL_toString_METHOD_OVERRIDE
};

#ifdef VENUS_INCLUDE_GLTF
class GLTF_Node : public Node {
public:
  using Ptr = hermes::Ref<GLTF_Node>;

  struct ImageData {
    mem::AllocatedImage image;
    mem::Image::View view;
  };

  static Result<Ptr> from(const std::filesystem::path &path,
                          const engine::GraphicsDevice &gd);

  ~GLTF_Node() noexcept;

  void draw(const hermes::geo::Transform &top_matrix,
            DrawContext &ctx) override;
  void destroy() noexcept override;

private:
  // named data

  std::unordered_map<std::string, ImageData> images_;
  std::unordered_map<std::string, Model::Ptr> meshes_;
  std::unordered_map<std::string, Model::Storage<mem::AllocatedBuffer>>
      mesh_storage_;
  std::unordered_map<std::string, Node::Ptr> nodes_;
  std::unordered_map<std::string, Material::Instance::Ptr> materials_;

  // top nodes on the GLTF tree
  std::vector<Node::Ptr> top_nodes_;

  // constructed data

  std::vector<Sampler> samplers_;
  std::vector<mem::Image::Handle> image_handles_;

  pipeline::DescriptorAllocator descriptor_allocator_;

  mem::AllocatedBuffer material_data_buffer_;

  VENUS_to_string_FRIEND(GLTF_Node);
  VENUS_VIRTUAL_toString_METHOD_OVERRIDE
};
#endif // VENUS_INCLUDE_GLTF

#ifdef VENUS_INCLUDE_VDB
class VDB_Node : public Node {
public:
  using Ptr = hermes::Ref<VDB_Node>;

  static Result<Ptr> from(const std::filesystem::path &vdb_file_path,
                          const engine::GraphicsDevice &gd);

  ~VDB_Node() noexcept;

  void draw(const hermes::geo::Transform &top_matrix,
            DrawContext &ctx) override;
  void destroy() noexcept override;

private:
  mem::AllocatedBuffer gpu_vdb_data_;
  AllocatedModel bounds_model_;

  pipeline::DescriptorAllocator descriptor_allocator_;
  Material::Instance::Ptr material_;
  mem::AllocatedBuffer material_data_buffer_;

  VENUS_to_string_FRIEND(VDB_Node);
  VENUS_VIRTUAL_toString_METHOD_OVERRIDE
};
#endif // VENUS_INCLUDE_VDB

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
  LabeledGraph &addModel(const std::string &name, const Model::Ptr &model,
                         const std::string &parent = "");
  LabeledGraph &addCamera(const std::string &name, const Camera::Ptr &camera,
                          const std::string &parent = "");
  template <typename NodeType = Node>
  const NodeType *get(const std::string &name) const {
    auto it = nodes_.find(name);
    if (it == nodes_.end()) {
      HERMES_WARN("Node {} not found in scene graph.", name);
      return {};
    }
    return reinterpret_cast<const NodeType *>(it->second.get());
  }
  template <typename NodeType = Node> NodeType *get(const std::string &name) {
    auto it = nodes_.find(name);
    if (it == nodes_.end()) {
      HERMES_WARN("Node {} not found in scene graph.", name);
      return {};
    }
    return reinterpret_cast<NodeType *>(it->second.get());
  }

  void draw(const hermes::geo::Transform &top_matrix,
            DrawContext &context) override;
  void destroy() noexcept override;

private:
  std::unordered_map<std::string, Node::Ptr> nodes_;

  VENUS_to_string_FRIEND(LabeledGraph);
  VENUS_VIRTUAL_toString_METHOD_OVERRIDE
};

} // namespace graph

} // namespace venus::scene
