#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba32f) uniform image2D output_image;
layout(binding = 1, rgba32f) uniform image2D accumulation_image;
layout(binding = 2) uniform sampler3D u_voxelData;
layout(binding = 3) uniform sampler3D u_voxelNormals;

uniform vec2    u_resolution;
uniform vec3    u_cameraPosition;
uniform mat4    u_viewMatrix;
uniform int		u_frameCount;
uniform float	u_time;

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

		// Convert integer voxel coordinate to texture lookup coordinate
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

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	if (pixel_coords.x >= int(u_resolution.x) || pixel_coords.y >= int(u_resolution.y))
		return;

	// Compute normalized device coordinates
	vec2 uv = (vec2(pixel_coords) / u_resolution) * 2.0 - 1.0;
	uv.x *= (u_resolution.x / u_resolution.y);

	// Build a perspective ray in view space.
	float fov = 90.0;
	float focal_length = 1.0 / tan(radians(fov) / 2.0);
	vec3 view_space_ray = normalize(vec3(uv.x, uv.y, -focal_length));

	Ray ray;
	ray.origin    = u_cameraPosition;
	ray.direction = normalize((inverse(u_viewMatrix) * vec4(view_space_ray, 0.0)).xyz);

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
