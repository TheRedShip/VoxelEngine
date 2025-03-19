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
	_camera = new Camera(glm::vec3(static_cast<float>(VOXEL_DIM / 2.0)), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f);
}

Scene::~Scene()
{
	delete (_camera);
}

struct Voxel
{
	uint8_t r, g, b, a;
};

struct VoxChunk
{
	int width, height, depth;
	std::vector<std::vector<std::vector<Voxel>>> voxels;
};

struct VoxModel
{
	int width, height, depth;
	std::vector<VoxChunk> chunks;
};

bool parseVoxFile(const std::string& filename, VoxModel& model)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		std::cerr << "Failed to open file: " << filename << std::endl;
		return false;
	}

	char header[4];
	file.read(header, 4);
	if (std::strncmp(header, "VOX ", 4) != 0) {
		std::cerr << "Invalid VOX file format." << std::endl;
		return false;
	}
	
	uint32_t version;
	file.read(reinterpret_cast<char*>(&version), 4);
	

	while (file) {
		char chunkId[4];
		file.read(chunkId, 4);
		
		uint32_t chunkSize, childChunks;
		file.read(reinterpret_cast<char*>(&chunkSize), 4);
		file.read(reinterpret_cast<char*>(&childChunks), 4);
		
		std::string chunk(chunkId, 4);
		
		std::cout << "Chunk " << chunk << " " << chunkSize << " " << childChunks << std::endl;

		if (chunk == "SIZE")
		{
			model.chunks.push_back(VoxChunk());
			
			VoxChunk &chunk = model.chunks.back();
			
			file.read(reinterpret_cast<char*>(&chunk.width), 4);
			file.read(reinterpret_cast<char*>(&chunk.height), 4);
			file.read(reinterpret_cast<char*>(&chunk.depth), 4);
			chunk.voxels.resize(chunk.depth, std::vector<std::vector<Voxel>>(chunk.height, std::vector<Voxel>(chunk.width)));
		}
		else if (chunk == "XYZI") {
			VoxChunk &chunk = model.chunks.back();

			uint32_t numVoxels;
			file.read(reinterpret_cast<char*>(&numVoxels), 4);
			
			for (uint32_t i = 0; i < numVoxels; ++i) {
				uint8_t x, y, z, colorIndex;
				file.read(reinterpret_cast<char*>(&x), 1);
				file.read(reinterpret_cast<char*>(&y), 1);
				file.read(reinterpret_cast<char*>(&z), 1);
				file.read(reinterpret_cast<char*>(&colorIndex), 1);
				
				chunk.voxels[z][y][x] = {colorIndex, colorIndex, colorIndex, 255};
			}
		}
		else if (chunk == "nTRN")
		{
			uint32_t nodeID;
			file.read(reinterpret_cast<char*>(&nodeID), 4);
			
			uint32_t dictSize;
			file.read(reinterpret_cast<char*>(&dictSize), 4);

			uint32_t childID;
			file.read(reinterpret_cast<char*>(&childID), 4);

			uint32_t reserved;
			file.read(reinterpret_cast<char*>(&reserved), 4);

			uint32_t layerID;
			file.read(reinterpret_cast<char*>(&layerID), 4);

			uint32_t numFrames;
			file.read(reinterpret_cast<char*>(&numFrames), 4);
		}
		else
			file.seekg(chunkSize, std::ios::cur);
	}
	
	model.width = 0;
	model.height = 0;
	model.depth = 0;
	
	for (VoxChunk &chunk : model.chunks)
	{
		model.width = std::max(model.width, chunk.width);
		model.height = std::max(model.height, chunk.height);
		model.depth = std::max(model.depth, chunk.depth);
	}
	
	return true;
}

void Scene::parseScene(std::string &name)
{
	std::ifstream	file(name);
	std::string		line;

	std::vector<unsigned char> voxelData(VOXEL_DIM * VOXEL_DIM * VOXEL_DIM * 4, 0);
	memset(voxelData.data(), 0, voxelData.size());

	if (file.is_open())
	{
		VoxModel model;
		if (parseVoxFile(name, model))
		{
			glm::ivec3 offset = glm::ivec3(0.);
			for (VoxChunk &chunk : model.chunks)
			{
				std::cout << "New voxel chunk " << chunk.width << "x" << chunk.height << "x" << chunk.depth << std::endl;
				
				for (int z = 0; z < chunk.depth; ++z)
				{
					for (int y = 0; y < chunk.height; ++y)
					{
						for (int x = 0; x < chunk.width; ++x)
						{
							int index_data = 4 * ((x + offset.x) + VOXEL_DIM * ((y + offset.y) + VOXEL_DIM * (z + offset.z)));						
							
							Voxel voxel = chunk.voxels[z][y][x];
							voxelData[index_data + 0] = voxel.r;
							voxelData[index_data + 1] = voxel.g;
							voxelData[index_data + 2] = voxel.b;
							voxelData[index_data + 3] = voxel.a;
						}
					}
				}
			}
		}
	}
	
	// Create and upload the 3D texture at texture unit index 2
	GLuint voxelTex;
	glGenTextures(1, &voxelTex);
	glActiveTexture(GL_TEXTURE1);  // Bind to texture unit 2
	glBindTexture(GL_TEXTURE_3D, voxelTex);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, VOXEL_DIM, VOXEL_DIM, VOXEL_DIM, 0, GL_RGBA, GL_UNSIGNED_BYTE, voxelData.data());
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void		Scene::addMaterial(Material *material)
{
	GPUMaterial	gpu_mat;

	gpu_mat.color = material->color;
	gpu_mat.emission = material->emission;
	gpu_mat.roughness = material->roughness;
	gpu_mat.metallic = material->metallic;
	gpu_mat.refraction = material->refraction;
	gpu_mat.type = material->type;
	gpu_mat.texture_index = material->texture_index;
	gpu_mat.emission_texture_index = material->emission_texture_index;

	_gpu_materials.push_back(gpu_mat);
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
