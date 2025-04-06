#extension GL_NV_gpu_shader5 : enable

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;

uniform vec2    u_resolution;

struct GPUVoxel
{
	vec3 normal;
	ivec3 position;

	int color;
	
	uint light_x;
	uint light_y;
	uint light_z;
	uint accum_count;
};

struct GPUFlatVoxel
{
	ivec3	pos;

	uint64_t 	child_mask;
	uint32_t 	child_offset;
	
	uint32_t 	voxel_index;
	uint32_t 	voxel_count;
	
	int			scale;
};

layout(std430, binding = 1) buffer VoxelFlatData
{
	GPUVoxel flatVoxels[];
};

layout(std430, binding = 2) buffer VisibleVoxelData
{
	uint32_t	voxel_per_pixels[SHADER_WIDTH * SHADER_HEIGHT];
	uint32_t 	visible_voxel_index[SHADER_WIDTH * SHADER_HEIGHT];
	uint32_t 	visible_voxel_flags[(SHADER_WIDTH * SHADER_HEIGHT) / 32];
	uint32_t 	visible_voxel_count;
};

#include "shaders/color.glsl"

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y))
		return ;
	
	uint32_t voxel_index = voxel_per_pixels[pixel_coords.x + pixel_coords.y * SHADER_WIDTH];
	if (voxel_index == 0)
	{
		imageStore(output_image, pixel_coords, vec4(1.));
		return ;
	}
		
	GPUVoxel voxel = flatVoxels[voxel_index];
	vec3 voxel_color = unpack_color(voxel.color).rgb;
	vec3 voxel_light = vec3(flatVoxels[voxel_index].light_x / 255.0, 
						   flatVoxels[voxel_index].light_y / 255.0,
						   flatVoxels[voxel_index].light_z / 255.0) / float(flatVoxels[voxel_index].accum_count);
	// voxel_light = vec3(1.);
	imageStore(output_image, pixel_coords, vec4(voxel_color * voxel_light, 1.0));
}