
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;

uniform vec2    u_resolution;
uniform int		u_frameCount;
uniform float	u_time;
uniform int		u_voxelDim;
uniform float	u_voxelSize;

struct GPUVoxel
{
	ivec3 position;
	int color;
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

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct hitInfo
{
	vec4 color;
	vec3 position;
	vec3 normal;
	float dist;
};

#include "shaders/random.glsl"
#include "shaders/svo.glsl"

vec3 pathtrace(Ray ray, inout uint rng_state)
{
	Stats stats;

	vec3 color = vec3(1.);

	vec3 light_dir = normalize(vec3(sin(u_time), -0.5, 0.));

	for (int i = 0; i < 1; i++)
	{
		hitInfo hit;
		if (!traverseSVO(ray, hit, stats))
		{
			color *= vec3(0.2, 0.4, 1.0);
			break;
		}
		
		color *= hit.color.rgb;
		// color *= hit.normal;
		// break ;

		//shadow ray//
		Ray shadow_ray;
		shadow_ray.origin = hit.position + hit.normal;
		shadow_ray.direction = -light_dir;

		hitInfo temp;
		if (traverseSVO(shadow_ray, temp, stats))
			color.rgb *= 0.5;
		//
		
		float diffuse = max(dot(hit.normal, -light_dir), 0.1);
		color *= diffuse;
	}
	
	return (color);
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

	return (Ray(origin, ray_direction));
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

	vec3 color = pathtrace(ray, rng_state);

	imageStore(output_image, pixel_coords, vec4(sqrt(color), 1.0));
}
