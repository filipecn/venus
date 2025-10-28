# Getting Started

![ubuntu/gcc](https://github.com/filipecn/hermes/actions/workflows/gcc_compiler.yml/badge.svg)
[![Coverage Status](https://coveralls.io/repos/github/filipecn/hermes/badge.svg?branch=main)](https://coveralls.io/github/filipecn/hermes?branch=main)
[![License:MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub](https://badgen.net/badge/icon/github?icon=github&label)](https://github.com/filipecn/hermes)

\tableofcontents

Welcome to Hermes! A multi-purpose library that may (hopefully) help you quickly set up a 
`C++`/`CUDA` project by providing you a bunch of auxiliary tools and structures. 

> This library is my personal lib I use in my projects, at my own risk :) Please keep it in mind. But
> I really hope it can be useful to you too.

## Features
Although most of `hermes` classes live in a single namespace, you will find the files organized 
in folders, such as: `geometry`, `data_structures`, `storage`, etc. You can find examples and
documentation for each of these groups here:


  // allocate ub for triangle model
  u32 buffer_index = 0;
  VENUS_ASSIGN_OR_RETURN_BAD_RESULT(
      buffer_index, cache.buffers().allocate(
                        "ub", sizeof(venus::scene::Material_Test::Data)));



  VENUS_RETURN_BAD_RESULT(
      scene.addNewMaterial<venus::scene::Material_BindlessTest>(
          "bindless", venus::engine::GraphicsEngine::device()));


  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::AllocatedModel, triangle_model_ptr,
      venus::scene::AllocatedModel::Config::fromShape(
          venus::scene::shapes::triangle, //
          hermes::geo::point3(-2.f, 0.f, 0.f),
          hermes::geo::point3(-2.f, 0.5f, 0.5f),
          hermes::geo::point3(-2.f, 0.f, 0.5f),
          venus::scene::shape_option_bits::none)
          .create(venus::engine::GraphicsEngine::device()));



  // update triangle material parameters
  const auto *material = scene.getMaterial("bindless");
  venus::scene::Material_Test parameters;
  // parameters.data.projection =
  //     hermes::math::transpose(camera.projectionTransform().matrix());
  // parameters.data.view =
  //     hermes::math::transpose(camera.viewTransform().matrix());
  // parameters.resources.data_buffer_offset = 0;
  // VENUS_ASSIGN_OR_RETURN_BAD_RESULT(parameters.resources.data_buffer,
  //                                          cache.allocatedBuffers()["ub"]);
  //// write parameters to uniform buffer
  // VENUS_RETURN_BAD_RESULT(cache.allocatedBuffers().copyBlock(
  //     "ub", 0, &parameters.data, sizeof(venus::scene::Material_Test::Data)));

  // write material descriptor set
  VENUS_DECLARE_SHARED_PTR_FROM_RESULT_OR_RETURN_BAD_RESULT(
      venus::scene::Material::Instance, mat,
      parameters.write(renderer.descriptorAllocator(), material));

  // update model material
  auto *triangle_node =
      scene.graph().get<venus::scene::graph::ModelNode>("triangle");
  triangle_node->model()->setMaterial(0, mat);


  // global ub
  // test/bindless test ub
  VENUS_RETURN_BAD_RESULT(
      cache.buffers().addBuffer("ub",
                                venus::mem::AllocatedBuffer::Config::forUniform(
                                    sizeof(venus::scene::Material_Test::Data)),
                                *gd));




| group | description |
|--------------|--------|
| [common](3_common.md) | auxiliary classes iterating, strings, files, argument parsing, etc  |
| [logging](4_logging.md) | logging messages  |
| [debugging](5_debugging.md) | debugging utilities and macros  |
| [profiling](6_profiling.md) | time profiling tools  |
| [numeric](7_numeric.md) | math operations, interpolation, intervals   |
| [geometry](8_geometry.md) | geometry related objects, functions and utilities such as vector, point, matrix, transforms, intersection tests, line, plane, etc   |
| [storage](10_storage.md) | memory classes: allocators, memory blocks, array of structs, etc  |
| [CUDA](9_cuda.md) | CUDA utilites and integration  |

## Download 

You can find Hermes in [github](https://github.com/filipecn/hermes).

> Please check the [build](1_build_and_install.md) and [link](2_linking.md) instructions to learn
how to build and use `hermes` into your project.

For the impatient, here is what you can do:
```shell
git clone https://github.com/filipecn/hermes.git
cd hermes
mkdir build
cd build
cmake .. -DINSTALL_PATH=/my/install/location
make -j8 install
```
and to compile along with your code here is what to do:
```shell
g++ -I/my/install/location/include                \
    -l/my/install/location/lib/libhermes.a         \
    main.cpp  
```

## Contact

Please feel free to contact me by [e-mail](mailto:filipedecn@gmail.com) :)
