
#extension GL_NV_gpu_shader5 : enable

layout(local_size_x = 16, local_size_y = 16) in;

uniform vec2    u_resolution;
uniform float	u_voxelSize;
uniform int		u_frameCount;

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

struct GPUCamera
{
	mat4	view_matrix;
    vec3	position;
	
	float	aperture_size;
	float	focus_distance;
	float	fov;

	int		bounce;
};

layout(std430, binding = 0) buffer FlatSVONode
{
	GPUFlatVoxel flatSVONodes[];
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

layout(std140, binding = 0) uniform CameraData
{
    GPUCamera camera;
};

struct Ray
{
	vec3 origin;
	vec3 direction;
	vec3 inv_direction;
};

struct hitInfo
{
	int voxel_index;
	vec3 position;
};

#include "shaders/svo.glsl"

Ray initRay(vec2 uv)
{
	float focal_length = 1.0 / tan(radians(camera.fov) / 2.0);
	
	vec3 origin = camera.position / u_voxelSize;
	vec3 view_space_ray = normalize(vec3(uv.x, uv.y, -focal_length));
	vec3 ray_direction = normalize((inverse(camera.view_matrix) * vec4(view_space_ray, 0.0)).xyz);
	
	vec3 right = vec3(camera.view_matrix[0][0], camera.view_matrix[1][0], camera.view_matrix[2][0]);
	vec3 up = vec3(camera.view_matrix[0][1], camera.view_matrix[1][1], camera.view_matrix[2][1]);

	return (Ray(origin, ray_direction, 1.0 / ray_direction));
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y))
		return;

	uint rng_state = uint(u_resolution.x) * uint(pixel_coords.y) + uint(pixel_coords.x);
	rng_state = rng_state + u_frameCount * 719393;

	vec2 uv = ((vec2(pixel_coords)) / u_resolution) * 2.0 - 1.0;
	uv.x *= u_resolution.x / u_resolution.y;

	Ray ray = initRay(uv);
	hitInfo hit = { -1, vec3(0.0) };
	Stats stats = { 0, 0 };
	if (traverseSVO(ray, hit, stats))
	{
		voxel_per_pixels[pixel_coords.x + pixel_coords.y * SHADER_WIDTH] = hit.voxel_index;

		uint word_index = hit.voxel_index / 32;
    	uint bit_mask = 1u << (hit.voxel_index % 32);
		
		uint old_value = atomicOr(visible_voxel_flags[word_index], bit_mask);

		if ((old_value & bit_mask) == 0u)
		{
			uint32_t current_index = atomicAdd(visible_voxel_count, 1);
			visible_voxel_index[current_index] = hit.voxel_index;
		}
	}
}