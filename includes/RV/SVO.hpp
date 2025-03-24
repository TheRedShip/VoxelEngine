/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SVO.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 15:40:14 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/21 15:40:14 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SVO_HPP
# define SVO_HPP

#include "RV.hpp"

struct FlatSVONode
{
	alignas(16) glm::ivec3 min;
	alignas(16) glm::ivec3 max;
    int childOffset;   // If not a leaf, the index where the children start in the flat node array.
    int voxelIndex;    // If a leaf, the index into the voxel array.
    int voxelCount;    // Number of voxels stored in this leaf.
    uint8_t childMask; // Bits 0-7: each bit indicates existence of a child.
};

class GPUVoxel;

class SVO
{
	public:
		SVO(glm::ivec3 min, glm::ivec3 max);
		~SVO();

		bool insert(GPUVoxel &voxel, int depth);
		bool contains(GPUVoxel &voxel);
		void subdivide();

		void flatten(std::vector<FlatSVONode> &flatNodes, std::vector<GPUVoxel> &flatVoxels);

		void print(int level);
		
		bool isLeaf();

		int	getNodeCount();
		


	private:
		SVO *_children[8];

		std::vector<GPUVoxel> _voxels;

		glm::ivec3 _min;
		glm::ivec3 _max;

		bool _empty;
		bool _leaf;

};

#endif