#extension GL_NV_gpu_shader5 : enable

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;

uniform vec2    u_resolution;
uniform int		u_frameCount;
uniform float	u_time;
uniform int		u_voxelDim;
uniform float	u_voxelSize;

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

struct GPUDebug
{
	int	enabled;
	int	mode;
	int	voxel_treshold;
	int	box_treshold;
};


layout(std430, binding = 0) buffer FlatSVONode
{
	GPUFlatVoxel flatSVONodes[];
};

layout(std430, binding = 1) buffer VoxelFlatData
{
	GPUVoxel flatVoxels[];
};

layout(std140, binding = 0) uniform CameraData
{
    GPUCamera camera;
};

layout(std140, binding = 1) uniform DebugData
{
    GPUDebug debug;
};

struct Ray {
	vec3 origin;
	vec3 direction;
	vec3 inv_direction;
};

struct hitInfo
{
	vec3 position;
	uint voxel_index;
	float dist;
};

#include "shaders/random.glsl"
#include "shaders/svo.glsl"

vec3 debugColor(Ray ray)
{
	Stats stats = Stats(0, 0);
	hitInfo hit;


	if (traverseSVO(ray, hit, stats))
		return (vec3(1.));
	return (vec3(0.));
	
	float node_display = float(stats.nodes) / float(debug.box_treshold);
	float voxel_display = float(stats.voxels) / float(debug.voxel_treshold);

	GPUVoxel voxel = flatVoxels[hit.voxel_index];

	switch (debug.mode)
	{
		case 0:
			return (voxel.normal);
		case 1:
			return (node_display < 1. ? vec3(node_display) : vec3(1., 0., 0.));
		case 2:
			return (voxel_display < 1. ? vec3(voxel_display) : vec3(1., 0., 0.));
	}

	return (vec3(0.));
}

Ray initRay(vec2 uv, inout uint rng_state)
{
	float focal_length = 1.0 / tan(radians(camera.fov) / 2.0);
	
	vec3 origin = camera.position / u_voxelSize;
	vec3 view_space_ray = normalize(vec3(uv.x, uv.y, -focal_length));
	vec3 ray_direction = normalize((inverse(camera.view_matrix) * vec4(view_space_ray, 0.0)).xyz);
	
	vec3 right = vec3(camera.view_matrix[0][0], camera.view_matrix[1][0], camera.view_matrix[2][0]);
	vec3 up = vec3(camera.view_matrix[0][1], camera.view_matrix[1][1], camera.view_matrix[2][1]);

	vec3 focal_point = origin + ray_direction * camera.focus_distance;

	float r = sqrt(randomValue(rng_state));
	float theta = 2.0 * M_PI * randomValue(rng_state);
	vec2 lens_point = camera.aperture_size * r * vec2(cos(theta), sin(theta));

	origin += right * lens_point.x + up * lens_point.y;
	ray_direction = normalize(focal_point - origin);

	return (Ray(origin, ray_direction, 1.0 / ray_direction));
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y))
		return;

	uint rng_state = uint(u_resolution.x) * uint(pixel_coords.y) + uint(pixel_coords.x);
	rng_state = rng_state + u_frameCount * 719393;

	vec2 jitter = randomPointInCircle(rng_state) * 1;

	vec2 uv = ((vec2(pixel_coords) + jitter) / u_resolution) * 2.0 - 1.0;
	uv.x *= u_resolution.x / u_resolution.y;

	Ray ray = initRay(uv, rng_state);

	vec3 color = debugColor(ray);

	imageStore(output_image, pixel_coords, vec4(color, 1.0));
}
