/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SVO.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 20:39:19 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/21 20:39:19 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SVO.hpp"

SVO::SVO(glm::ivec3 min, glm::ivec3 max)
{
	_min = min;
	_max = max;
	
	memset(_children, 0, sizeof(_children));

	_leaf = true;
	_empty = true;
}

SVO::~SVO()
{
	for (int i = 0; i < 8; i++)
	{
		if (_children[i])
			delete _children[i];
	}
}

bool SVO::insert(GPUVoxel &voxel, int depth)
{
	if (!this->contains(voxel))
		return (false);

	_empty = false;

	if (depth == 0)
	{
		_voxels.push_back(voxel);
		return (true);
	}

	if (_leaf && _voxels.size() < 8)
	{
		_voxels.push_back(voxel);
		return (true);
	}

	if (_leaf)
		this->subdivide();

	for (int i = 0; i < 8; i++)
	{
		if (_children[i]->insert(voxel, depth - 1))
			return (true);
	}

	return (false);
}

void SVO::subdivide()
{
	glm::ivec3 mid = (_min + _max) / 2;

	_children[0] = new SVO(glm::ivec3(_min.x, _min.y, _min.z), glm::ivec3(mid.x, mid.y, mid.z));
	_children[1] = new SVO(glm::ivec3(mid.x, _min.y, _min.z), glm::ivec3(_max.x, mid.y, mid.z));
	_children[2] = new SVO(glm::ivec3(_min.x, mid.y, _min.z), glm::ivec3(mid.x, _max.y, mid.z));
	_children[3] = new SVO(glm::ivec3(mid.x, mid.y, _min.z), glm::ivec3(_max.x, _max.y, mid.z));
	_children[4] = new SVO(glm::ivec3(_min.x, _min.y, mid.z), glm::ivec3(mid.x, mid.y, _max.z));
	_children[5] = new SVO(glm::ivec3(mid.x, _min.y, mid.z), glm::ivec3(_max.x, mid.y, _max.z));
	_children[6] = new SVO(glm::ivec3(_min.x, mid.y, mid.z), glm::ivec3(mid.x, _max.y, _max.z));
	_children[7] = new SVO(glm::ivec3(mid.x, mid.y, mid.z), glm::ivec3(_max.x, _max.y, _max.z));

	for (GPUVoxel &voxel : _voxels)
	{
		for (int i = 0; i < 8; i++)
		{
			if (_children[i]->contains(voxel))
			{
				_children[i]->insert(voxel, 0);
				break;
			}
		}
	}

	_voxels.clear();
	_leaf = false;
}

bool SVO::contains(GPUVoxel &voxel)
{
	return (voxel.position.x >= _min.x && voxel.position.x < _max.x &&
		voxel.position.y >= _min.y && voxel.position.y < _max.y &&
		voxel.position.z >= _min.z && voxel.position.z < _max.z);
}

void SVO::flatten(std::vector<FlatSVONode> &flatNodes, std::vector<GPUVoxel> &flatVoxels)
{
	flatNodes.push_back(FlatSVONode{});

	FlatSVONode &rootNode = flatNodes[0];
	
	rootNode.min = _min;
	rootNode.max = _max;
	rootNode.childMask = 0;
	rootNode.voxelCount = 0;
	rootNode.voxelIndex = -1;
	rootNode.childOffset = -1;
	
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
			if (!currentNode->_voxels.empty())
			{
				flatNode.voxelIndex = flatVoxels.size();
				flatNode.voxelCount = currentNode->_voxels.size();
				
				for (GPUVoxel &voxel : currentNode->_voxels)
					flatVoxels.push_back(voxel);
			}
		}
		else
		{
			uint32_t childMask = 0;
			
			for (int i = 0; i < 8; i++)
			{
				if (!currentNode->_children[i]->_empty)
					childMask |= 1 << i;
			}
			
			flatNode.childOffset = flatNodes.size();
			flatNode.childMask = childMask;
			
			// Reserve space for all children
			int startIndex = flatNodes.size();
			flatNodes.resize(startIndex + 8, {}); // Add 8 empty nodes
			
			// Initialize children and add them to the queue
			for (int i = 0; i < 8; i++)
			{
				int childFlatIndex = startIndex + i;
				
				// Initialize default values
				FlatSVONode& childFlatNode = flatNodes[childFlatIndex];
				childFlatNode.voxelIndex = -1;
				childFlatNode.childOffset = -1;
				childFlatNode.voxelCount = 0;
				childFlatNode.childMask = 0;
				
				if (currentNode->_children[i])
				{
					// Set proper values
					childFlatNode.min = currentNode->_children[i]->_min;
					childFlatNode.max = currentNode->_children[i]->_max;
					
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

    // Print node info: whether it's a leaf and number of voxels stored.
    std::cout << indent << "SVO Node (" << (_leaf ? "Leaf" : "Internal") 
              << "), Voxel count: " << _voxels.size() << "\n";

    // Optionally, print voxel positions if the node is a leaf.
    if (_leaf) {
        for (const auto &voxel : _voxels) {
            std::cout << indent << "  Voxel: (" 
                      << voxel.position.x << ", " 
                      << voxel.position.y << ", " 
                      << voxel.position.z << ")\n";
        }
    }
    
    // Recursively print children nodes if internal.
    if (!_leaf) {
        for (int i = 0; i < 8; i++) {
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



