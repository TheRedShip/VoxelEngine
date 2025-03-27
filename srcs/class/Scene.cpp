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

	_gpu_debug.enabled = 0;
	_gpu_debug.mode = 0;
	_gpu_debug.triangle_treshold = 1;
	_gpu_debug.box_treshold = 1;
}

Scene::~Scene()
{
	delete (_camera);
}

void	Scene::placeModel(VoxModel &model, glm::ivec3 position, std::vector<GPUVoxel> &voxel_data)
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
						voxel_data[index_data].color = color;
					}
				}
			}
		}
	}
}

void Scene::parseScene(std::string &name)
{
	SVO *root = new SVO(glm::ivec3(0), glm::ivec3(VOXEL_DIM));

	std::vector<GPUVoxel> voxel_data;
	voxel_data.resize(VOXEL_DIM * VOXEL_DIM * VOXEL_DIM);
	memset(voxel_data.data(), 0, voxel_data.size());

	VoxModel model = VoxModel(name);
	if (model.isParsed())
		this->placeModel(model, glm::ivec3(VOXEL_DIM / 2), voxel_data);
	else
		std::cout << "Failed to parse vox model" << std::endl;
	
	// int index_data = (120 + VOXEL_DIM * (120 + VOXEL_DIM * 120));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (10 + VOXEL_DIM * (120 + VOXEL_DIM * 120));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (120 + VOXEL_DIM * (10 + VOXEL_DIM * 120));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (120 + VOXEL_DIM * (120 + VOXEL_DIM * 10));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (10 + VOXEL_DIM * (10 + VOXEL_DIM * 10));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (10 + VOXEL_DIM * (120 + VOXEL_DIM * 10));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (120 + VOXEL_DIM * (10 + VOXEL_DIM * 10));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (10 + VOXEL_DIM * (10 + VOXEL_DIM * 120));
	// voxel_data[index_data].color = 0x00FF00FF;

	// index_data = (30 + VOXEL_DIM * (30 + VOXEL_DIM * 30));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (5 + VOXEL_DIM * (30 + VOXEL_DIM * 30));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (30 + VOXEL_DIM * (5 + VOXEL_DIM * 30));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (30 + VOXEL_DIM * (30 + VOXEL_DIM * 5));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (5 + VOXEL_DIM * (5 + VOXEL_DIM * 5));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (5 + VOXEL_DIM * (30 + VOXEL_DIM * 5));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (30 + VOXEL_DIM * (5 + VOXEL_DIM * 5));
	// voxel_data[index_data].color = 0x00FF00FF;
	// index_data = (5 + VOXEL_DIM * (5 + VOXEL_DIM * 30));
	// voxel_data[index_data].color = 0x00FF00FF;

	// for (int x = 0; x < 30; x++)
	// {
	// 	index_data = (x + VOXEL_DIM * (30 + VOXEL_DIM * 30));
	// 	voxel_data[index_data].color = 0xFF0000FF;
	// }
	for (int z = 0; z < VOXEL_DIM; ++z)
	{
		for (int y = 0; y < VOXEL_DIM; ++y)
		{
			for (int x = 0; x < VOXEL_DIM; ++x)
			{
				int index_data = (x + VOXEL_DIM * (y + VOXEL_DIM * z));
			
				if (y <= 2)
					voxel_data[index_data].color = 0x00FF00FF;

			}
		}
	}

	//count time to insert voxels in ms
	auto start = std::chrono::high_resolution_clock::now();

	int count = 0;
	for (int z = 0; z < VOXEL_DIM; ++z)
	{
		for (int y = 0; y < VOXEL_DIM; ++y)
		{
			for (int x = 0; x < VOXEL_DIM; ++x)
			{
				int index_data = (x + VOXEL_DIM * (y + VOXEL_DIM * z));
			
				if (y <= 2)
					voxel_data[index_data].color = 0x00FF00FF;

				if (voxel_data[index_data].color != 0)
				{
					count++;
					GPUVoxel voxel;
					voxel.position = glm::ivec3(x, y, z);
					voxel.color = voxel_data[index_data].color;
					voxel.normal = glm::vec3(0.);

					for (int xo = -1; xo <= 1; xo++)
					{
						for (int yo = -1; yo <= 1; yo++)
						{
							for (int zo = -1; zo <= 1; zo++)
							{
								glm::ivec3 offset = glm::ivec3(xo, yo, zo);

								int new_index_data = (x + xo + VOXEL_DIM * ((y + yo) + VOXEL_DIM * (z + zo)));
								if (new_index_data < 0 || new_index_data >= VOXEL_DIM * VOXEL_DIM * VOXEL_DIM)
									continue;

								if (voxel_data[new_index_data].color == 0)
									voxel.normal += glm::vec3(offset);
							}
						}
					}

					voxel.normal = glm::normalize(voxel.normal);

					root->insert(voxel, 16);
				}
			}
		}
	}
	
	root->flatten(flatNodes, flatVoxels);

	std::cout << "Voxels inserted: " << count << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms" << std::endl;

	// for (int i = 0; i < flatNodes.size(); i++)
	// {
	// 	std::cout << "Node: " << i << std::endl;
	// 	std::cout << "Min: " << flatNodes[i].min.x << " " << flatNodes[i].min.y << " " << flatNodes[i].min.z << std::endl;
	// 	std::cout << "Max: " << flatNodes[i].max.x << " " << flatNodes[i].max.y << " " << flatNodes[i].max.z << std::endl;
	// 	std::cout << "Child offset: " << flatNodes[i].childOffset << std::endl;
	// 	std::cout << "Voxel index: " << flatNodes[i].voxelIndex << std::endl;
	// 	std::cout << "Voxel count: " << flatNodes[i].voxelCount << std::endl;
	// 	std::cout << "Child mask: " << (int)flatNodes[i].childMask << std::endl;
	// 	for (int j = 0; j < flatNodes[i].voxelCount; j++)
	// 	{
	// 		std::cout << "Voxel: " << flatVoxels[flatNodes[i].voxelIndex + j].position.x << " " << flatVoxels[flatNodes[i].voxelIndex + j].position.y << " " << flatVoxels[flatNodes[i].voxelIndex + j].position.z << std::endl;
	// 	}
	// 	std::cout << std::endl;
	// }

	// root->print(0);
	voxel_data.clear();
}

void		Scene::addMaterial(GPUMaterial material)
{
	_gpu_materials.push_back(material);
}

std::vector<GPUMaterial>		&Scene::getMaterialData()
{
	return (_gpu_materials);
}

GPUDebug	&Scene::getDebug(void)
{
	return (_gpu_debug);
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
