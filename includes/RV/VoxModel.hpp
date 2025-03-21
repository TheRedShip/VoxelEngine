/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Voxel.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 22:55:17 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/20 22:55:17 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VOXEL_HPP
# define VOXEL_HPP

#include "RV.hpp"

struct Voxel
{
	bool active;
	uint8_t paletteIndex;
};

struct VoxChunk
{
	int width, height, depth;
    int offset_x, offset_y, offset_z;
	std::vector<std::vector<std::vector<Voxel>>> voxels;
};

class VoxModel
{
	public:
		VoxModel(std::string &name);
		~VoxModel();

		static bool parseVoxFile(const std::string& filename, VoxModel& model);

		const glm::vec3			&getSize() const;

		std::vector<VoxChunk>	&getChunks();
		const uint32_t			*getPalette() const;

		const bool				&isParsed() const;

		void					setSize(glm::vec3 size);
		void					setPalette(uint32_t palette[256]);


	private:
		bool					_parsed;

		glm::vec3				_size;

		std::vector<VoxChunk>	_chunks;

		bool					_hasPalette;
		uint32_t				_palette[256];
};

#endif