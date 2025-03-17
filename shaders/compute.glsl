
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;
layout(binding = 1) uniform sampler3D u_voxelData;
layout(binding = 2) uniform sampler3D u_voxelNormals;

uniform vec2    u_resolution;
uniform int		u_frameCount;
uniform float	u_time;


struct GPUCamera
{
	mat4	view_matrix;
    vec3	position;
	
	float	aperture_size;
	float	focus_distance;
	float	fov;

	int		bounce;
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
	vec3 position;
	vec3 normal;
	vec4 color;
};

#include "shaders/random.glsl"

bool voxelRayMarch(Ray ray, out hitInfo hit)
{
	const vec3 voxelDim   = vec3(128.0);

	vec3 tDelta = vec3(0.0);
	vec3 tMax   = vec3(0.0);
	ivec3 steps = ivec3(0);

	for (int i = 0; i < 3; i++)
	{
		tDelta[i] = 1.0 / max(abs(ray.direction[i]), 0.001);
		steps[i] = int(sign(ray.direction[i]));
		if (ray.direction[i] > 0.0)
		{
			float voxelBoundary = floor(ray.origin[i]) + 1.0;
			tMax[i] = (voxelBoundary - ray.origin[i]) * tDelta[i];
		}
		else
		{
			float voxelBoundary = floor(ray.origin[i]);
			tMax[i] = (ray.origin[i] - voxelBoundary) * tDelta[i];
		}
	}

	ivec3 ipos = ivec3(floor(ray.origin));
	
	float tHit;

	for (int i = 0; i < 128 * 3; i++)
	{
		if (ipos.x < 0 || ipos.y < 0 || ipos.z < 0 || ipos.x >= 128 || ipos.y >= 128 || ipos.z >= 128)
			return (false);

		vec3 texCoords = (vec3(ipos) + 0.5) / voxelDim;
		vec4 voxelColor = texture(u_voxelData, texCoords);
		vec3 voxelNormal = texture(u_voxelNormals, texCoords).rgb;
		
		if (voxelColor.a != 0.0)
		{
			hit.color = voxelColor;
			hit.position = vec3(ipos) + 0.5;
			hit.normal = voxelNormal;
			return (true);
		}
		
		// Choose the next axis to step: x, y, or z
		if (tMax.x < tMax.y && tMax.x < tMax.z)
		{
			tHit = tMax.x;
			ipos.x += steps.x;
			tMax.x += tDelta.x;
		}
		else if (tMax.y < tMax.z)
		{
			tHit = tMax.y;
			ipos.y += steps.y;
			tMax.y += tDelta.y;
		}
		else
		{
			tHit = tMax.z;
			ipos.z += steps.z;
			tMax.z += tDelta.z;
		}
	}
	return (false);
}

Ray initRay(vec2 uv, inout uint rng_state)
{
	float focal_length = 1.0 / tan(radians(camera.fov) / 2.0);
	
	vec3 origin = camera.position;
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


	vec3 light_dir = normalize(vec3(sin(u_time), -1.0, 0.));
	
	hitInfo hit;
	if (voxelRayMarch(ray, hit))
	{
		ray.origin = hit.position + normalize(hit.normal);
		ray.direction = -light_dir;

		hitInfo temp;
		if (voxelRayMarch(ray, temp))
			hit.color.rgb *= 0.75;
		
		float diffuse = max(dot(hit.normal, -light_dir), 0.25);
		hit.color.rgb *= diffuse;
	}
	else
		hit.color = vec4(vec3(0.), 1.0);
	

	imageStore(output_image, pixel_coords, sqrt(hit.color));
}
