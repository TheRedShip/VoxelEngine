/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:29:41 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/19 17:17:44 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scene.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Scene::Scene()
{
	_camera = new Camera(glm::vec3(static_cast<float>((VOXEL_DIM / 2.0) * VOXEL_SIZE)), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f);
}

Scene::~Scene()
{
	delete (_camera);
}

void	Scene::placeModel(VoxModel &model, glm::ivec3 position)
{
	for (VoxChunk &chunk : model.getChunks())
	{
		std::cout << "New voxel chunk " << chunk.width << "x" << chunk.height << "x" << chunk.depth << std::endl;
		std::cout << "Offset " << chunk.offset_x << " " << chunk.offset_y << " " << chunk.offset_z << std::endl;

		glm::ivec3 offset = position + glm::ivec3(chunk.offset_x, chunk.offset_y, chunk.offset_z);
		
		for (int z = 0; z < chunk.depth; ++z)
		{
			for (int y = 0; y < chunk.height; ++y)
			{
				for (int x = 0; x < chunk.width; ++x)
				{
					if (x >= VOXEL_DIM || x < 0 || y >= VOXEL_DIM || y < 0 || z >= VOXEL_DIM || z < 0)
					{
						std::cout << "Voxel: " << x << " " << y << " " << z << " is out of bounds" << std::endl;
						continue;
					}
					int index_data = ((x + offset.x) + VOXEL_DIM * ((y + offset.y) + VOXEL_DIM * (z + offset.z)));						
					
					Voxel voxel = chunk.voxels[z][y][x];
					if (voxel.active)
					{
						uint32_t color = model.getPalette()[voxel.paletteIndex];
						_voxelData[index_data].color = color;
					}
				}
			}
		}
	}
}

void Scene::parseScene(std::string &name)
{
	_voxelData.resize(VOXEL_DIM * VOXEL_DIM * VOXEL_DIM);
	memset(_voxelData.data(), 0, _voxelData.size());

	VoxModel model = VoxModel(name);
	if (!model.isParsed())
	{
		std::cout << "Failed to parse vox model" << std::endl;
		return;
	}

	this->placeModel(model, glm::ivec3(VOXEL_DIM / 2));

    for (int z = 0; z < VOXEL_DIM; ++z)
    {
        for (int y = 0; y < VOXEL_DIM; ++y)
        {
            for (int x = 0; x < VOXEL_DIM; ++x)
            {
				int index_data = (x + VOXEL_DIM * (y + VOXEL_DIM * z));
                if (y <= 1)
                {
                    int red   = 0;
                    int green = 150 + (rand() % 25);
                    int blue  = 0;
                    int alpha = 255;

                    int packedColor = (red   << 24) |
                                      (green << 16) |
                                      (blue  << 8)  |
                                      (alpha);

					_voxelData[index_data].color = packedColor;
				}
            }
        }
    }
}

void		Scene::addMaterial(GPUMaterial material)
{
	_gpu_materials.push_back(material);
}

std::vector<GPUVoxel>		&Scene::getVoxelData()
{
	return (_voxelData);
}

std::vector<GPUMaterial>		&Scene::getMaterialData()
{
	return (_gpu_materials);
}

Camera							*Scene::getCamera(void) const
{
	return (_camera);
}

GPUMaterial	Scene::getMaterial(int material_index)
{
	if (material_index < 0 || material_index >= (int)_gpu_materials.size())
		throw std::runtime_error("Incorrect material index");
	return (_gpu_materials[material_index]);
}
