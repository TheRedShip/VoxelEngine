
struct Stats
{
	int nodes;
	int voxels;
};

bool intersectRayBox(Ray ray, vec3 box_min, vec3 box_max, inout float dist)
{
	vec3 t1 = (box_min - ray.origin) * ray.inv_direction;
	vec3 t2 = (box_max - ray.origin) * ray.inv_direction;
	
	vec3 tMin = min(t1, t2);
	vec3 tMax = max(t1, t2);
	
	dist = max(max(tMin.x, tMin.y), tMin.z);
	float last_dist = min(min(tMax.x, tMax.y), tMax.z);
	
	return (dist <= last_dist && last_dist >= 0.0);
}

bool traverseSVO(Ray ray, inout hitInfo hit, inout Stats stats)
{
	hit.dist = 1e30;

	int stack[32];
	int stack_ptr = 0;
	stack[0] = 0;

	while (stack_ptr >= 0)
	{
		int current_index = stack[stack_ptr--];
		GPUFlatVoxel node = flatSVONodes[current_index];
		
		if (node.childOffset == -1) // leaf
		{
			for (int i = 0; i < node.voxelCount; i++)
            {
                int index = node.voxelIndex + i;
                GPUVoxel voxel = flatVoxels[index];

				vec3 box_min = voxel.position;
				vec3 box_max = voxel.position + vec3(1.001);

				float dist = 0.;
				if (intersectRayBox(ray, box_min, box_max, dist) && dist < hit.dist)
				{
					hit.dist = dist;
					hit.voxel_index = index;
				}

				stats.voxels++;
            }
		}
		else
		{
			for (int i = 0; i < 8; i++)
			{
				if ((node.childMask & (1 << i)) != 0)
				{
					GPUFlatVoxel child = flatSVONodes[node.childOffset + i];

					float dist = 0.;
					if (intersectRayBox(ray, vec3(child.min), vec3(child.max), dist) && dist < hit.dist)
						stack[++stack_ptr] = node.childOffset + i;

					stats.nodes++;
				}
			}
		}
	}

	return (hit.dist < 1e30);
}