
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
	ivec3 min;
	ivec3 max;
    int childOffset;
    int voxelIndex;
    int voxelCount;

	uint childMask;
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
	float dist;
};

#include "shaders/random.glsl"
#include "shaders/svo.glsl"

vec3[2] pathtrace(Ray ray, inout uint rng_state, inout int voxel_index)
{
	Stats stats;

	vec3[2] color_light;
	color_light[0] = vec3(1.);
	color_light[1] = vec3(0.);

	for (int i = 0; i < 3; i++)
	{
		hitInfo hit;
		if (!traverseSVO(ray, hit, stats))
		{
			color_light[1] += vec3(1.);
			break;
		}

		if (i == 0)
			voxel_index = hit.voxel_index;
		
		GPUVoxel voxel = flatVoxels[hit.voxel_index];
		vec4 voxel_color = vec4(
			float((voxel.color >> 24u) & 0xFFu) / 255.0,
			float((voxel.color >> 16u) & 0xFFu) / 255.0,
			float((voxel.color >> 8u) & 0xFFu) / 255.0,
			float(voxel.color & 0xFFu) / 255.0);

		color_light[0] *= voxel_color.rgb;

		ray.origin = voxel.position + 0.5 + voxel.normal;
		ray.direction = randomHemisphereDirection(voxel.normal, rng_state);
		ray.inv_direction = 1.0 / ray.direction;
	}
	
	return (color_light);
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

vec4 unpack_color(uint packed_color)
{
    float r = float((packed_color >> 24u) & 0xFFu) / 255.0;
    float g = float((packed_color >> 16u) & 0xFFu) / 255.0;
    float b = float((packed_color >> 8u) & 0xFFu) / 255.0;
    float a = float(packed_color & 0xFFu) / 255.0;
    return vec4(r, g, b, a);
}

uint pack_color(vec3 color)
{
    uint r = uint(color.r * 255.0) & 0xFFu;
    uint g = uint(color.g * 255.0) & 0xFFu;
    uint b = uint(color.b * 255.0) & 0xFFu;
    uint a = 0xFFu;
    return (r << 24u) | (g << 16u) | (b << 8u) | a;
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y))
		return;

	uint rng_state = uint(u_resolution.x) * uint(pixel_coords.y) + uint(pixel_coords.x);
	rng_state = rng_state + u_frameCount * 719393;

	vec2 jitter = randomPointInCircle(rng_state) * 1;

	vec2 uv = ((vec2(pixel_coords)) / u_resolution) * 2.0 - 1.0;
	uv.x *= u_resolution.x / u_resolution.y;

	Ray ray = initRay(uv, rng_state);

	int voxel_index = -1;
	vec3[2] color_light = pathtrace(ray, rng_state, voxel_index);

	vec3 final_light;

	if (voxel_index != -1)
	{
		if (flatVoxels[voxel_index].accum_count == 0)
		{
			atomicExchange(flatVoxels[voxel_index].light_x, 0);
			atomicExchange(flatVoxels[voxel_index].light_y, 0);
			atomicExchange(flatVoxels[voxel_index].light_z, 0);
		}
		
		if (flatVoxels[voxel_index].accum_count < 20000)
		{
			atomicAdd(flatVoxels[voxel_index].light_x, int(color_light[1].x * color_light[0].x * 255.));
			atomicAdd(flatVoxels[voxel_index].light_y, int(color_light[1].y * color_light[0].y * 255.));
			atomicAdd(flatVoxels[voxel_index].light_z, int(color_light[1].z * color_light[0].z * 255.));

			atomicAdd(flatVoxels[voxel_index].accum_count, 1);
		}

		final_light = vec3(flatVoxels[voxel_index].light_x / 255.0, 
						   flatVoxels[voxel_index].light_y / 255.0,
						   flatVoxels[voxel_index].light_z / 255.0) / float(flatVoxels[voxel_index].accum_count);
	}
	else
		final_light = color_light[1];


	imageStore(output_image, pixel_coords, vec4(final_light, 1.0));
}
