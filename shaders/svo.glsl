
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

bool IsSolidVoxelAt(ivec3 pos)
{
    uint currentIndex = 0;
    
    while (true)
    {
        GPUFlatVoxel node = flatSVONodes[currentIndex];
        
        if (!inBounds(pos, node.pos, node.scale))
            return (false);
        
        if (node.child_mask == 0) // leaf
        {
            ivec3 localPos = pos - node.pos;
            int index = localPos.x + localPos.y * 8 + localPos.z * 8 * 8;
            
            if (index < 0 || index >= node.voxel_count)
                return false;
            
            return (flatVoxels[node.voxel_index + index].color != 0);
        }
        else
        {
            int childScale = node.scale / 4;

            ivec3 rel = (pos - node.pos) / childScale;

            rel.x = clamp(rel.x, 0, 3);
            rel.y = clamp(rel.y, 0, 3);
            rel.z = clamp(rel.z, 0, 3);
            
            uint childIndex = rel.x + rel.y * 4 + rel.z * 16;
            
            if ((node.child_mask & (1ul << childIndex)) == 0ul)
                return (false);
            
            uint childNodeIndex = node.child_offset + childIndex;
            currentIndex = childNodeIndex;
        }
    }
    
    return (false);
}

vec2 IntersectAABB(Ray ray, vec3 bbMin, vec3 bbMax)
{
    vec3 t0 = (bbMin - ray.origin) * ray.inv_direction;
    vec3 t1 = (bbMax - ray.origin) * ray.inv_direction;

    vec3 temp = t0;
    t0 = min(temp, t1), t1 = max(temp, t1);

    float tmin = max(max(t0.x, t0.y), t0.z);
    float tmax = min(min(t1.x, t1.y), t1.z);

    return vec2(tmin, tmax);
}

bool traverseSVO(Ray ray, inout hitInfo hit, inout Stats stats)
{
	hit.dist = 1e30;

    vec3 pos = ray.origin;
    float tmax = 0;

    for (int i = 0; i < 256; i++)
	{
        ivec3 voxelPos = ivec3(floor(pos));
        if (IsSolidVoxelAt(voxelPos)) return (true);

        vec3 cellMin = voxelPos;
        vec3 cellMax = cellMin + 1.0;
        vec2 time = IntersectAABB(ray, cellMin, cellMax);

        tmax = time.y + 0.0001;
        pos = ray.origin + tmax * ray.direction;

		stats.nodes++;
    }

	return (false);
}