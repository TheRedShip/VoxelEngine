
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;

uniform vec2    u_resolution;
uniform int		u_frameCount;
uniform float	u_time;
uniform int		u_voxelDim;
uniform float	u_voxelSize;

struct GPUVoxel
{
	int color;
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

layout(std430, binding = 0) buffer VoxelData
{
	GPUVoxel voxels[];
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

GPUVoxel readVoxel(ivec3 pos)
{
    if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= u_voxelDim || pos.y >= u_voxelDim || pos.z >= u_voxelDim)
        return GPUVoxel(0);
    
    int index = pos.x + u_voxelDim * (pos.y + u_voxelDim * pos.z);
	return voxels[index];
}

vec3 getGradientNormal(ivec3 pos)
{
	vec3 normal = vec3(0.);

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				ivec3 offset = ivec3(x, y, z);

				GPUVoxel voxel = readVoxel(pos + offset);
				float alpha = float(voxel.color & 0xFFu) / 255.0;

				if (alpha == 0)
					normal += offset;
			}
		}
	}

    return normalize(normal);
}


bool voxelRayMarch(Ray ray, out hitInfo hit)
{
	vec3 tDelta = vec3(0.0);
	vec3 tMax   = vec3(0.0);
	ivec3 steps = ivec3(0);

	ivec3 ipos = ivec3(floor(ray.origin / u_voxelSize));

	float cell = u_voxelSize;
	for (int i = 0; i < 3; i++)
	{
		tDelta[i] = cell / max(abs(ray.direction[i]), 0.001);
		steps[i] = int(sign(ray.direction[i]));
		if (ray.direction[i] > 0.0)
		{
			float voxelBoundary = (float(ipos[i]) + 1.0) * cell;
			tMax[i] = (voxelBoundary - ray.origin[i]) / abs(ray.direction[i]);
		}
		else
		{
			float voxelBoundary = float(ipos[i]) * cell;
			tMax[i] = (ray.origin[i] - voxelBoundary) / abs(ray.direction[i]);
		}
	}

	int axis = 0;

	for (int i = 0; i < u_voxelDim * 3; i++)
	{
		if (ipos.x < 0 || ipos.y < 0 || ipos.z < 0 || ipos.x >= u_voxelDim || ipos.y >= u_voxelDim || ipos.z >= u_voxelDim)
			return (false);

		GPUVoxel voxel = readVoxel(ipos);
		
		vec4 color = vec4(0.);
		color.r = float((voxel.color >> 24u) & 0xFFu) / 255.0;
		color.g = float((voxel.color >> 16u) & 0xFFu) / 255.0;
		color.b = float((voxel.color >> 8u)  & 0xFFu) / 255.0;
		color.a = float(voxel.color & 0xFFu) / 255.0;
		
		if (color.a != 0.0)
		{
			hit.color = color;
			hit.position = (vec3(ipos) + 0.5) * u_voxelSize;
			hit.normal = getGradientNormal(ipos);
			return (true);
		}
		
		if (tMax.x < tMax.y && tMax.x < tMax.z)
			axis = 0;
		else if (tMax.y < tMax.z)
			axis = 1;
		else
			axis = 2;

		ipos[axis] += steps[axis];
		tMax[axis] += tDelta[axis];
	}
	return (false);
}

#include "shaders/random.glsl"

vec3 pathtrace(Ray ray, inout uint rng_state)
{
	vec3 color = vec3(1.);

	vec3 light_dir = normalize(vec3(sin(u_time * 0.01), -0.5, 0.));

	for (int i = 0; i < 1; i++)
	{
		hitInfo hit;
		if (!voxelRayMarch(ray, hit))
		{
			color *= vec3(0.2, 0.4, 1.0);
			break;
		}
		
		color *= hit.color.rgb;
		// color *= hit.normal;
		// break ;

		//shadow ray//
		Ray shadow_ray;
		shadow_ray.origin = hit.position + hit.normal * u_voxelSize;
		shadow_ray.direction = -light_dir;

		hitInfo temp;
		if (voxelRayMarch(shadow_ray, temp))
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

	vec3 color = pathtrace(ray, rng_state);

	imageStore(output_image, pixel_coords, vec4(sqrt(color), 1.0));
}
