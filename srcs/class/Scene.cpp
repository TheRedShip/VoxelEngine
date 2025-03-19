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
	bool active;
	uint8_t paletteIndex;
};

struct VoxChunk
{
	int width, height, depth;
    int offset_x, offset_y, offset_z;
	std::vector<std::vector<std::vector<Voxel>>> voxels;
};

struct VoxModel
{
	int width, height, depth;
	bool hasPalette;
	uint32_t palette[256];
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

	bool has_palette = false;
	
	for (int i = 0; i < 256; ++i)
	{
		uint8_t r, g, b, a;
		r = i; g = i; b = i; a = 255;
		model.palette[i] = (r << 24) | (g << 16) | (b << 8) | a;
	}
	
	while (file) {
		char chunkId[4];
		file.read(chunkId, 4);
		
		uint32_t chunkSize, childChunks;
		file.read(reinterpret_cast<char*>(&chunkSize), 4);
		file.read(reinterpret_cast<char*>(&childChunks), 4);
		
		std::string chunk(chunkId, 4);

		if (chunk == "SIZE")
		{
			model.chunks.push_back(VoxChunk());
			
			VoxChunk &chunk = model.chunks.back();
			
			file.read(reinterpret_cast<char*>(&chunk.width), 4);
			file.read(reinterpret_cast<char*>(&chunk.depth), 4);
			file.read(reinterpret_cast<char*>(&chunk.height), 4);
			chunk.voxels.resize(chunk.depth, std::vector<std::vector<Voxel>>(chunk.height, std::vector<Voxel>(chunk.width)));
		}
		else if (chunk == "XYZI") {
			VoxChunk &chunk = model.chunks.back();

			uint32_t numVoxels;
			file.read(reinterpret_cast<char*>(&numVoxels), 4);
			
			for (uint32_t i = 0; i < numVoxels; ++i) {
				uint8_t x, y, z, colorIndex;
				file.read(reinterpret_cast<char*>(&x), 1);
				file.read(reinterpret_cast<char*>(&z), 1);
				file.read(reinterpret_cast<char*>(&y), 1);
				file.read(reinterpret_cast<char*>(&colorIndex), 1);
				
				chunk.voxels[z][y][x].active = true;
				chunk.voxels[z][y][x].paletteIndex = colorIndex;
			}
		}
		else if (chunk == "nTRN")
		{
			uint32_t nodeID;
			file.read(reinterpret_cast<char*>(&nodeID), 4);
			
			uint32_t dictSize;
			file.read(reinterpret_cast<char*>(&dictSize), 4);

			for (uint32_t i = 0; i < dictSize; i++)
			{
				//just skip dont read
				uint32_t keySize;
				file.read(reinterpret_cast<char*>(&keySize), 4);
				file.seekg(keySize, std::ios::cur);

				uint32_t valueSize;
				file.read(reinterpret_cast<char*>(&valueSize), 4);
				file.seekg(valueSize, std::ios::cur);
			}

			uint32_t childID;
			file.read(reinterpret_cast<char*>(&childID), 4);

			uint32_t reserved;
			file.read(reinterpret_cast<char*>(&reserved), 4);

			uint32_t layerID;
			file.read(reinterpret_cast<char*>(&layerID), 4);

			uint32_t numFrames;
			file.read(reinterpret_cast<char*>(&numFrames), 4);

			std::cout << "Node ID: " << nodeID << " dictSize " << dictSize  << " Child ID: " << childID << " Reserved " << reserved << " Layer ID: " << layerID << " Num Frames: " << numFrames << std::endl;

			for (uint32_t i = 0; i < numFrames; i++)
            {
                // dict frame attributes

                uint32_t dictFrameSize;
                file.read(reinterpret_cast<char*>(&dictFrameSize), 4);

                for (uint32_t j = 0; j < dictFrameSize; j++)
                {
                    uint32_t keySize;
                    file.read(reinterpret_cast<char*>(&keySize), 4);

                    std::string key;
                    key.resize(keySize);
                    file.read(key.data(), keySize);

                    uint32_t valueSize;
                    file.read(reinterpret_cast<char*>(&valueSize), 4);

                    std::string value;
                    value.resize(valueSize);
                    file.read(value.data(), valueSize);

                    if (key == "_t")
                    {
                        VoxChunk &chunk = model.chunks[(nodeID / 2) - 1];
                        
                        std::stringstream values(value);
                        values >> chunk.offset_x >> chunk.offset_z >> chunk.offset_y;
                    }
                    else if (key == "_r")
                        file.seekg(sizeof(uint8_t), std::ios::cur);
                    else if (key == "_f")
                        file.seekg(sizeof(uint32_t), std::ios::cur);
                }

            } 
		}
		else if (chunk == "RGBA")
		{
			if (has_palette)
			{
				file.seekg(chunkSize, std::ios::cur);
				continue;
			}
			
			has_palette = true;

			for (uint32_t i = 0; i < 256; ++i)
			{
				uint8_t r, g, b, a;
				file.read(reinterpret_cast<char*>(&r), 1);
				file.read(reinterpret_cast<char*>(&g), 1);
				file.read(reinterpret_cast<char*>(&b), 1);
				file.read(reinterpret_cast<char*>(&a), 1);

				a = 255;
				std::cout << "Color " << i << ": " << (int)r << " " << (int)g << " " << (int)b << " " << (int)a << std::endl;
				
				model.palette[i] = (r << 24) | (g << 16) | (b << 8) | a;
			}
		}
		else
			file.seekg(chunkSize, std::ios::cur);
	}
	
	int minX = std::numeric_limits<int>::max();
	int minY = std::numeric_limits<int>::max();
	int minZ = std::numeric_limits<int>::max();
	int maxX = std::numeric_limits<int>::min();
	int maxY = std::numeric_limits<int>::min();
	int maxZ = std::numeric_limits<int>::min();

	for (VoxChunk &chunk : model.chunks)
	{
		minX = std::min(minX, chunk.offset_x);
		minY = std::min(minY, chunk.offset_y);
		minZ = std::min(minZ, chunk.offset_z);
		
		maxX = std::max(maxX, chunk.offset_x + chunk.width);
		maxY = std::max(maxY, chunk.offset_y + chunk.height);
		maxZ = std::max(maxZ, chunk.offset_z + chunk.depth);
	}

	model.width  = maxX - minX;
	model.height = maxY - minY;
	model.depth  = maxZ - minZ;
	
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
			std::cout << "Vox model parsed successfully" << std::endl;
			std::cout << "Model size: " << model.width << "x" << model.height << "x" << model.depth << std::endl;
            for (VoxChunk &chunk : model.chunks)
			{
                std::cout << "New voxel chunk " << chunk.width << "x" << chunk.height << "x" << chunk.depth << std::endl;
                std::cout << "Offset " << chunk.offset_x << " " << chunk.offset_y << " " << chunk.offset_z << std::endl;

                glm::ivec3 offset = glm::ivec3(VOXEL_DIM / 2) + glm::ivec3(chunk.offset_x, chunk.offset_y, chunk.offset_z);
                
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
							int index_data = 4 * ((x + offset.x) + VOXEL_DIM * ((y + offset.y) + VOXEL_DIM * (z + offset.z)));						
							
							Voxel voxel = chunk.voxels[z][y][x];
							if (voxel.active)
							{
								uint32_t color = model.palette[voxel.paletteIndex];
								voxelData[index_data + 0] = (color >> 24) & 0xFF;
								voxelData[index_data + 1] = (color >> 16) & 0xFF;
								voxelData[index_data + 2] = (color >> 8) & 0xFF;
								voxelData[index_data + 3] = color & 0xFF;
							}
						}
					}
				}
			}
		}
	}

    for (int z = 0; z < VOXEL_DIM; ++z)
    {
        for (int y = 0; y < VOXEL_DIM; ++y)
        {
            for (int x = 0; x < VOXEL_DIM; ++x)
            {
                if (y == 0)
                {
                    int index_data = 4 * (x + VOXEL_DIM * (y + VOXEL_DIM * z));
                    voxelData[index_data + 0] = 0;
                    voxelData[index_data + 1] = 100 + rand() % 25;
                    voxelData[index_data + 2] = 0;
                    voxelData[index_data + 3] = 255;
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
