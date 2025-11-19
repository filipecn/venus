#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_GOOGLE_include_directive : require

layout(set = 0, binding = 0) uniform  
SceneData {   
	
  mat4 view;
	mat4 projection;
	vec3 eye;

} scene_data;

layout(buffer_reference, scalar) readonly buffer 
PNanoVDBBuffer { 

  // raw nanovdb data
	uint data[];

};

layout(set = 1, binding = 0) uniform 
MaterialData {   

  PNanoVDBBuffer vdb;

} material_data;

#define pnanovdb_buf_data material_data.vdb.data
#define PNANOVDB_GLSL
#include "PNanoVDB.h"

#include "venus/debug.glsl"
#include "venus/geometry.glsl"

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 frag_wp;

struct VolumeNano {
  pnanovdb_buf_t          buf;
  pnanovdb_grid_handle_t  grid;
  pnanovdb_grid_type_t    grid_type;
  pnanovdb_readaccessor_t accessor;
  vec3                    bbox_min;
  vec3                    bbox_max;
} volume_nano;

void initialize_volume_nano() {
  volume_nano.buf.unused = 0; // Unused because we create a buffer from device address
  volume_nano.grid.address.byte_offset = 0;
  volume_nano.grid_type = pnanovdb_buf_read_uint32(volume_nano.buf, PNANOVDB_GRID_OFF_GRID_TYPE);

  pnanovdb_tree_handle_t tree = pnanovdb_grid_get_tree(volume_nano.buf, volume_nano.grid);
  pnanovdb_root_handle_t root = pnanovdb_tree_get_root(volume_nano.buf, tree);
  pnanovdb_readaccessor_init(volume_nano.accessor, root);

  volume_nano.bbox_min = pnanovdb_root_get_bbox_min(volume_nano.buf, root);
  volume_nano.bbox_max = pnanovdb_root_get_bbox_max(volume_nano.buf, root);

}

float sampleMediumDensity(vec3 position) {
  vec3 index_space_position = pnanovdb_grid_world_to_indexf(volume_nano.buf, volume_nano.grid, position);
  pnanovdb_coord_t ijk = pnanovdb_hdda_pos_to_ijk(index_space_position);
  pnanovdb_address_t address = pnanovdb_readaccessor_get_value_address(
      volume_nano.grid_type,
      volume_nano.buf, 
      volume_nano.accessor, 
      ijk
      );

  return pnanovdb_read_float(volume_nano.buf, address);
}

void clipRay(inout Ray ray) {
  pnanovdb_hdda_ray_clip(
    volume_nano.bbox_min,
    volume_nano.bbox_max,
    ray.o,
    ray.t_min,
    ray.d,
    ray.t_max
  );

  if(ray.t_min > ray.t_max && ray.t_max < 0) {
    ray.t_min *= -1;
    ray.t_max *= -1;
  }
}

//#include "venus/volume.glsl"

void main()
{
  initialize_volume_nano();

  //uint seed = tea(uint(gl_FragCoord.x), uint(gl_FragCoord.y));

  //Medium medium;
  //medium.sigma_a = vec3(0.5);
  //medium.sigma_s = vec3(0.5);
  //medium.Le = vec3(0.5);

  Ray ray;
  ray.o = scene_data.eye;
  ray.d = normalize(frag_wp - scene_data.eye);
  ray.t_max = 1000;
  ray.t_min = 0;

  clipRay(ray);

  print_frag(ray.t_min);
  print_frag(ray.t_max);

  //vec3 L = solveRTE(medium, ray, 1, seed);

  //print_frag(L);
  
  vec3 p = ray.o + 2.0 * ray.d;
  outColor = vec4(p,2.0);

  float density = sampleMediumDensity(p);
  
  outColor = vec4(density, density,density, density);
}

