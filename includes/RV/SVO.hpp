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

# include "RV.hpp"

# define LEAF_SIZE 8

struct FlatSVONode
{
	alignas(16) glm::ivec3	pos;

	uint64_t 	child_mask;
	uint32_t 	child_offset;
	
	uint32_t 	voxel_index;
	uint32_t 	voxel_count;
	
	int			scale;
};

struct GPUVoxel
{
	alignas(16) glm::vec3 normal;
	alignas(16)	glm::ivec3 position;
	
	int color;

	uint32_t light_x;
	uint32_t light_y;
	uint32_t light_z;

	uint32_t accum_count;
};

class SVO
{
	public:
		SVO(glm::ivec3 pos, int scale, bool leaf); // internal nodes
		~SVO();

		void	insert(GPUVoxel &voxel);
		void	flatten(std::vector<FlatSVONode> &flatNodes, std::vector<GPUVoxel> &flatVoxels);

		void	print(int level);
		
		bool	isLeaf();
		int		getNodeCount();
		


	private:
		bool 		_leaf;

		SVO			*_children[64];
		uint64_t	_child_mask;

		GPUVoxel	_voxels[512]; // 8x8x8 leaf size
		uint32_t	_voxel_count;

		glm::ivec3	_pos;
		int			_scale;


};

#endif