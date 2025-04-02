
struct Stats
{
	int nodes;
	int voxels;
};

bool leafDDA(GPUFlatVoxel leaf, vec3 origin, vec3 direction, inout hitInfo hit, inout Stats stats)
{
    ivec3 currentVoxel = ivec3(floor(origin / u_voxelSize));

    ivec3 steps = ivec3(0);
    vec3 tDelta = vec3(0.0);
    vec3 tMax = vec3(0.0);

	for (int i = 0; i < 3; i++)
	{
		tDelta[i] = u_voxelSize / max(abs(direction[i]), 0.001);
		steps[i] = int(sign(direction[i]));
		if (direction[i] > 0.0)
		{
			float voxelBoundary = (float(currentVoxel[i]) + 1.0) * u_voxelSize;
			tMax[i] = (voxelBoundary - origin[i]) / abs(direction[i]);
		}
		else
		{
			float voxelBoundary = float(currentVoxel[i]) * u_voxelSize;
			tMax[i] = (origin[i] - voxelBoundary) / abs(direction[i]);
		}
	}

    int axis = 0;

    for (int i = 0; i < 100; i++)
	{
        stats.voxels++;
        
        if (currentVoxel.x < 0 || currentVoxel.y < 0 || currentVoxel.z < 0 ||
            currentVoxel.x >= 8 || currentVoxel.y >= 8 || currentVoxel.z >= 8)
            return (false);

        int index = currentVoxel.x + currentVoxel.y * 8 + currentVoxel.z * 8 * 8;

        if (index < 0 || index >= leaf.voxel_count)
            return false;

        if (flatVoxels[leaf.voxel_index + index].color != 0)
        {
            hit.voxel_index = int(leaf.voxel_index) + index;
            hit.position = vec3(currentVoxel) * u_voxelSize + vec3(0.5) * u_voxelSize;
            return (true);
        }

		if (tMax.x < tMax.y && tMax.x < tMax.z)
			axis = 0;
		else if (tMax.y < tMax.z)
			axis = 1;
		else
			axis = 2;

		currentVoxel[axis] += steps[axis];
		tMax[axis] += tDelta[axis];
	}

    return (false);
}

struct stackDDA
{
    vec3 origin;
    vec3 tDelta;
    vec3 tMax;
    ivec3 pos;
    ivec3 steps;
    int node_index;
    int axis;
};

stackDDA getStackDDA(vec3 origin, vec3 direction, int node_index)
{
    GPUFlatVoxel node = flatSVONodes[node_index];

    float node_size = node.scale * (u_voxelSize / 4.0);
    ivec3 current_node = ivec3(floor(origin / node_size));

    ivec3 steps = ivec3(0);
    vec3 tDelta = vec3(0.0);
    vec3 tMax = vec3(0.0);

    for (int i = 0; i < 3; i++)
	{
		tDelta[i] = node_size / max(abs(direction[i]), 0.0001);
		steps[i] = int(sign(direction[i]));
		if (direction[i] > 0.0)
		{
			float voxel_boundary = (float(current_node[i]) + 1.0) * node_size;
			tMax[i] = (voxel_boundary - origin[i]) / max(abs(direction[i]), 0.0001);
		}
		else
		{
			float voxel_boundary = float(current_node[i]) * node_size;
			tMax[i] = (origin[i] - voxel_boundary) / max(abs(direction[i]), 0.0001);
		}
	}

    return stackDDA(origin, tDelta, tMax, current_node, steps, node_index, 0);
}

bool traverseSVO(Ray ray, inout hitInfo hit, inout Stats stats)
{
    stackDDA stacks[4];
    int stack_ptr = 0;

    stacks[stack_ptr] = getStackDDA(ray.origin, ray.direction, 0);

    while (stack_ptr >= 0)
    {
        stackDDA stack = stacks[stack_ptr];
        int current_index = stack.node_index;
        GPUFlatVoxel node = flatSVONodes[current_index];

        if (node.child_mask == 0) // leaf (decrement stack_ptr)
        {
            if (leafDDA(node, stack.origin, ray.direction, hit, stats))
                return (true);
            stack_ptr--;
            continue;
        }

        bool found_child = false;

        for (int i = 0; i < 64; i++)
        {
            if (stack.pos.x < 0 || stack.pos.y < 0 || stack.pos.z < 0 ||
                stack.pos.x >= 4 || stack.pos.y >= 4 || stack.pos.z >= 4)
                break;

            int bitmask_index = stack.pos.x + stack.pos.y * 4 + stack.pos.z * 4 * 4;

            if ((node.child_mask & (1ul << bitmask_index)) != 0ul)
            {
                GPUFlatVoxel child = flatSVONodes[node.child_offset + bitmask_index];

                vec3 new_t = stack.tMax - stack.tDelta;
                float t = max(new_t[stack.axis], 0.);

                vec3 new_origin = stack.origin + ray.direction * t;
                new_origin += 0.001 * ray.direction; // Avoid self-intersection

                if (stack.tMax.x < stack.tMax.y && stack.tMax.x < stack.tMax.z)
                    stack.axis = 0;
                else if (stack.tMax.y < stack.tMax.z)
                    stack.axis = 1;
                else
                    stack.axis = 2;
                
                stack.pos[stack.axis] += stack.steps[stack.axis];
                stack.tMax[stack.axis] += stack.tDelta[stack.axis];

                stacks[stack_ptr] = stack; // Save updated parent state

                vec3 relative_child_pos = (vec3(child.pos) - vec3(node.pos)) * u_voxelSize;

                stackDDA child_stack = getStackDDA(new_origin - relative_child_pos, ray.direction, int(node.child_offset + bitmask_index));
                stacks[++stack_ptr] = child_stack;

                found_child = true;

                break ;
            }

            if (stack.tMax.x < stack.tMax.y && stack.tMax.x < stack.tMax.z)
                stack.axis = 0;
            else if (stack.tMax.y < stack.tMax.z)
                stack.axis = 1;
            else
                stack.axis = 2;

            stack.pos[stack.axis] += stack.steps[stack.axis];
            stack.tMax[stack.axis] += stack.tDelta[stack.axis];
        }

        if (!found_child)
            --stack_ptr;
    }
   
	return (false);
}

