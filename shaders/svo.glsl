
struct Stats
{
	int nodes;
	int voxels;
};

bool inBounds(ivec3 pos, ivec3 nodePos, int scale)
{
    return (pos.x >= nodePos.x && pos.y >= nodePos.y && pos.z >= nodePos.z &&
            pos.x < nodePos.x + scale && pos.y < nodePos.y + scale && pos.z < nodePos.z + scale);
}

int getNodeIndex(ivec3 pos, inout Stats stats)
{
    uint currentIndex = 0;
    
    while (true)
    {
        GPUFlatVoxel node = flatSVONodes[currentIndex];
        
        if (!inBounds(pos, node.pos, node.scale))
            return (-1);
        
        
        if (node.child_mask == 0) // leaf
            return int(currentIndex);
        else
        {
            int childScale = node.scale / 4;

            ivec3 rel = (pos - node.pos) / childScale;

            rel.x = clamp(rel.x, 0, 3);
            rel.y = clamp(rel.y, 0, 3);
            rel.z = clamp(rel.z, 0, 3);
            
            uint childIndex = rel.x + rel.y * 4 + rel.z * 16;
            
            if ((node.child_mask & (1ul << childIndex)) == 0ul)
                return (-1);
            
            uint childNodeIndex = node.child_offset + childIndex;
            currentIndex = childNodeIndex;

            stats.nodes++;
        }
    }
    
    return (-1);
}

bool IsSolidVoxelAt(ivec3 pos, inout Stats stats)
{
    int nodeIndex = getNodeIndex(pos, stats);
    
    if (nodeIndex == -1)
        return (false);

    GPUFlatVoxel node = flatSVONodes[nodeIndex];
    
    ivec3 localPos = pos - node.pos;
    int index = localPos.x + localPos.y * 8 + localPos.z * 8 * 8;
    
    if (index < 0 || index >= node.voxel_count)
        return false;
    
    return (flatVoxels[node.voxel_index + index].color != 0);
}

bool RayAABBIntersection(Ray ray, vec3 boxMin, float scale, inout float tEntry, inout float tExit)
{
    vec3 boxMax = boxMin + vec3(scale);
    
    vec3 t0s = (boxMin - ray.origin) * ray.inv_direction;
    vec3 t1s = (boxMax - ray.origin) * ray.inv_direction;

    vec3 tsmaller = min(t0s, t1s);
    vec3 tbigger  = max(t0s, t1s);
    
    tEntry = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    tExit  = min(min(tbigger.x, tbigger.y), tbigger.z);

    if (tEntry < 0)
        tEntry = 0.0f;
    
    return (tExit >= tEntry && tExit >= 0.0f);
}

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
            // hit.position = vec3(currentVoxel) * u_voxelSize + vec3(0.5) * u_voxelSize;
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

int treeDDA(GPUFlatVoxel node, vec3 origin, vec3 direction, inout Stats stats)
{
    ivec3 currentNode = ivec3(floor(origin));

    ivec3 steps = ivec3(0);
    vec3 tDelta = vec3(0.0);
    vec3 tMax = vec3(0.0);

    for (int i = 0; i < 3; i++)
	{
		tDelta[i] = u_voxelSize / max(abs(direction[i]), 0.001);
		steps[i] = int(sign(direction[i]));
		if (direction[i] > 0.0)
		{
			float voxelBoundary = (float(currentNode[i]) + 1.0) * u_voxelSize;
			tMax[i] = (voxelBoundary - origin[i]) / abs(direction[i]);
		}
		else
		{
			float voxelBoundary = float(currentNode[i]) * u_voxelSize;
			tMax[i] = (origin[i] - voxelBoundary) / abs(direction[i]);
		}
	}

    for (int i = 0; i < 100; i++)
    {
        stats.nodes++;

        if (currentNode.x < 0 || currentNode.y < 0 || currentNode.z < 0 ||
            currentNode.x >= 4 || currentNode.y >= 4 || currentNode.z >= 4)
            return (false);

        int bitmask_index = currentNode.x + currentNode.y * 4 + currentNode.z * 4 * 4;

        if (index < 0 || index >= 64)
            return (false);

        if ((node.child_mask & (1ul << bitmask_index)) != 0ul)
        {
            return (bitmask_index);
        }
    }
}


struct stackSVO
{
    int index;
    float tEntry;
};

bool traverseSVO(Ray ray, inout hitInfo hit, inout Stats stats)
{
    hit.dist = 1e30;

	stackSVO stack[32];
	int stack_ptr = 0;
	stack[0] = stackSVO(0, 0.0);

	while (stack_ptr >= 0) 
	{
		stackSVO current_stack = stack[stack_ptr--];
        
        int current_index = current_stack.index;
		GPUFlatVoxel node = flatSVONodes[current_index];
		
		if (node.child_mask == 0) // leaf
		{
			vec3 leaf_origin = ray.origin + ray.direction * current_stack.tEntry;
            vec3 localOrigin = (leaf_origin - vec3(node.pos)) * u_voxelSize;

            localOrigin += 0.0001 * ray.direction; // Avoid self-intersection

            if (leafDDA(node, localOrigin, ray.direction, hit, stats))
                return (true);
		}
		else
		{
			for (int i = 0; i < 64; i++)
			{
				if ((node.child_mask & (1ul << i)) != 0ul)
				{
					GPUFlatVoxel child = flatSVONodes[node.child_offset + i];

					float dist = 0.;
                    float tEntry = 0.;
                    float tExit = 0.;
					if (RayAABBIntersection(ray, child.pos, child.scale, tEntry, tExit))
						stack[++stack_ptr] = stackSVO(int(node.child_offset + i), tEntry);

					stats.nodes++;
				}
			}
		}
	}

	return (false);
}