/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SVO.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 20:39:19 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/28 16:00:38 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SVO.hpp"

SVO::SVO(glm::ivec3 pos, int scale, bool leaf) : _leaf(leaf), _child_mask(0), _pos(pos), _scale(scale)
{
	memset(_voxels, 0, sizeof(_voxels));
	memset(_children, 0, sizeof(_children));
}

SVO::~SVO()
{
	for (int i = 0; i < 8; i++)
	{
		if (_children[i])
			delete _children[i];
	}
}

void SVO::insert(GPUVoxel &voxel)
{
	if (_leaf)
	{
		glm::ivec3 local_pos = voxel.position - _pos;

		if (local_pos.x < 0 || local_pos.y < 0 || local_pos.z < 0 || local_pos.x >= LEAF_SIZE || local_pos.y >= LEAF_SIZE || local_pos.z >= LEAF_SIZE)
		{
			std::cerr << "Voxel out of bounds: " << voxel.position.x << " " << voxel.position.y << " " << voxel.position.z << std::endl;
			return ;
		}

		int index = local_pos.x + local_pos.y * LEAF_SIZE + local_pos.z * LEAF_SIZE * LEAF_SIZE;
		_voxels[index] = voxel;

		_voxel_count++;

		return ;
	}

	// 4x4x4 division
	int child_scale = _scale / 4;
	glm::ivec3 relative = (voxel.position - _pos) / child_scale;
	relative = glm::clamp(relative, glm::ivec3(0), glm::ivec3(3)); // Ensure it's within bounds

	int child_index = relative.x + relative.y * 4 + relative.z * 16; // 4x4x4 grid

	glm::ivec3 childPos = _pos + glm::ivec3(relative.x * child_scale,
												relative.y * child_scale,
												relative.z * child_scale);

	if (_children[child_index] == nullptr)
	{
		bool is_leaf = child_scale <= LEAF_SIZE;
		_children[child_index] = new SVO(childPos, child_scale, is_leaf);

		_child_mask |= (1ULL << child_index);
	}

	_children[child_index]->insert(voxel);
}

void SVO::flatten(std::vector<FlatSVONode> &flatNodes, std::vector<GPUVoxel> &flatVoxels)
{
	flatNodes.push_back(FlatSVONode{});

	FlatSVONode &rootNode = flatNodes[0];
	
	rootNode.pos = _pos;
	rootNode.scale = _scale;
	rootNode.child_mask = 0;
	rootNode.child_offset = 0;
	rootNode.voxel_index = 0;
	rootNode.voxel_count = 0;
	
	std::queue<std::pair<SVO *, int>> nodeQueue;
	nodeQueue.push({this, 0});
	
	while (!nodeQueue.empty())
	{
		auto [currentNode, currentIndex] = nodeQueue.front();
		nodeQueue.pop();
		
		FlatSVONode &flatNode = flatNodes[currentIndex];
		
		if (currentNode->_leaf)
		{
			// Handle leaf node
			if (currentNode->_voxel_count != 0)
			{
				flatNode.voxel_index = flatVoxels.size();
				flatNode.voxel_count = currentNode->_voxel_count;
				
				for (GPUVoxel &voxel : currentNode->_voxels)
					flatVoxels.push_back(voxel);
			}
		}
		else
		{
			uint64_t child_mask = 0;
			
			for (int i = 0; i < 64; i++)
			{
				if (currentNode->_children[i])
					child_mask |= 1 << i;
			}
			
			flatNode.child_offset = flatNodes.size();
			flatNode.child_mask = child_mask;
			
			int startIndex = flatNodes.size();
			flatNodes.resize(startIndex + 64, {}); // Add 8 empty nodes
			
			// Initialize children and add them to the queue
			for (int i = 0; i < 64; i++)
			{
				int childFlatIndex = startIndex + i;
				
				// Initialize default values
				FlatSVONode& childFlatNode = flatNodes[childFlatIndex];
				childFlatNode.voxel_index = 0;
				childFlatNode.voxel_count = 0;
				childFlatNode.child_offset = 0;
				childFlatNode.child_mask = 0;
				
				if (currentNode->_children[i])
				{
					// Set proper values
					childFlatNode.pos = currentNode->_children[i]->_pos;
					childFlatNode.scale = currentNode->_children[i]->_scale;
					
					// Add to queue for processing
					nodeQueue.push({currentNode->_children[i], childFlatIndex});
				}
			}
		}
	}
}

int SVO::getNodeCount()
{
	int count = 1;

	if (!_leaf)
	{
		for (int i = 0; i < 8; i++)
		{
			if (_children[i])
				count += _children[i]->getNodeCount();
		}
	}

	return count;
}

void SVO::print(int level)
{
    std::string indent(level * 4, ' ');

	int voxel_count = 0;
	for (int i = 0; i < LEAF_SIZE * LEAF_SIZE * LEAF_SIZE; i++)
		if (_voxels[i].color != 0)
			voxel_count++;

    // Print node info: whether it's a leaf and number of voxels stored.
    std::cout << indent << "SVO Node (" << (_leaf ? "Leaf" : "Internal") 
              << "), Voxel count: " << voxel_count << " at position ("
			  << _pos.x << ", " << _pos.y << ", " << _pos.z << "), Scale: "
			  << _scale << " and child mask: " << std::bitset<64>(_child_mask) << "\n";

    // Optionally, print voxel positions if the node is a leaf.
    if (_leaf) {
        for (const auto &voxel : _voxels)
		{
			if (voxel.color != 0)
            	std::cout << indent << "  Voxel: (" 
                      << voxel.position.x << ", " 
                      << voxel.position.y << ", " 
                      << voxel.position.z << ")\n";
        }
    }
    
    // Recursively print children nodes if internal.
    if (!_leaf) {
        for (int i = 0; i < 64; i++) {
            if (_children[i]) {
                _children[i]->print(level + 1);
            }
        }
    }
}

bool SVO::isLeaf()
{
	return _leaf;
}



