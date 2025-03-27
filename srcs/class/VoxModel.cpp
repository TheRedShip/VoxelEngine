/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VoxModel.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 13:20:57 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/21 13:20:57 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VoxModel.hpp"

VoxModel::VoxModel(std::string &name)
{
	std::ifstream	file(name);
	std::string		line;

	_size = glm::vec3(0.0f);
	_hasPalette = false;
	_parsed = false;

	memset(_palette, 0, sizeof(_palette));

	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << name << std::endl;
		return ;
	}

	if (VoxModel::parseVoxFile(name, *this))
	{
		std::cout << "Vox model parsed successfully" << std::endl;
		std::cout << "Model size: " << _size.x << "x" << _size.y << "x" << _size.z << std::endl;

		_parsed = true;
	}
}

VoxModel::~VoxModel()
{
}

const bool	&VoxModel::isParsed() const
{
	return (_parsed);
}

glm::ivec3				&VoxModel::getSize()
{
	return (_size);
}

std::vector<VoxChunk>	&VoxModel::getChunks()
{
	return (_chunks);
}

const uint32_t	*VoxModel::getPalette() const
{
	return (_palette);
}

void VoxModel::setSize(glm::ivec3 size)
{
	_size = size;
}

void			VoxModel::setPalette(uint32_t palette[256])
{
	memcpy(_palette, palette, sizeof(_palette));
}

