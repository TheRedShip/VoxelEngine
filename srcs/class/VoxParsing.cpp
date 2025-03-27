/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VoxParsing.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: TheRed <TheRed@students.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/21 13:30:00 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/21 13:30:00 by TheRed           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VoxModel.hpp"

bool VoxModel::parseVoxFile(const std::string &filename, VoxModel &model)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file)
		return (false);

	char header[4];
	file.read(header, 4);
	if (std::strncmp(header, "VOX ", 4) != 0) {
		std::cerr << "Invalid VOX file format." << std::endl;
		return (false);
	}
	
	uint32_t version;
	file.read(reinterpret_cast<char*>(&version), 4);

	bool has_palette = false;
	uint32_t palette[256];
	
	for (int i = 0; i < 256; ++i)
	{
		uint8_t r, g, b, a;
		r = i; g = i; b = i; a = 255;
		palette[i] = (r << 24) | (g << 16) | (b << 8) | a;
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
			model.getChunks().push_back(VoxChunk());
			
			VoxChunk &chunk = model.getChunks().back();
			
			file.read(reinterpret_cast<char*>(&chunk.width), 4);
			file.read(reinterpret_cast<char*>(&chunk.depth), 4);
			file.read(reinterpret_cast<char*>(&chunk.height), 4);
			chunk.voxels.resize(chunk.depth, std::vector<std::vector<Voxel>>(chunk.height, std::vector<Voxel>(chunk.width)));
		}
		else if (chunk == "XYZI") {
			VoxChunk &chunk = model.getChunks().back();

			uint32_t numVoxels;
			file.read(reinterpret_cast<char*>(&numVoxels), 4);
			
			for (uint32_t i = 0; i < numVoxels; ++i) {
				uint8_t x, y, z, colorIndex;
				file.read(reinterpret_cast<char*>(&x), 1);
				file.read(reinterpret_cast<char*>(&z), 1);
				file.read(reinterpret_cast<char*>(&y), 1);
				file.read(reinterpret_cast<char*>(&colorIndex), 1);
				
				chunk.voxels[z][y][x].active = true;
				chunk.voxels[z][y][x].paletteIndex = colorIndex - 1;
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

			// std::cout << "Node ID: " << nodeID << " dictSize " << dictSize  << " Child ID: " << childID << " Reserved " << reserved << " Layer ID: " << layerID << " Num Frames: " << numFrames << std::endl;

			for (uint32_t i = 0; i < numFrames; i++)
            {
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
                        VoxChunk &chunk = model.getChunks()[(nodeID / 2) - 1];
                        
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

			for (uint32_t i = 0; i < 256; i++)
			{
				uint8_t r, g, b, a;
				file.read(reinterpret_cast<char*>(&r), 1);
				file.read(reinterpret_cast<char*>(&g), 1);
				file.read(reinterpret_cast<char*>(&b), 1);
				file.read(reinterpret_cast<char*>(&a), 1);

				palette[i] = (r << 24) | (g << 16) | (b << 8) | a;
			}
		}
		else
			file.seekg(chunkSize, std::ios::cur);
	}

	model.setPalette(palette);
	
	glm::ivec3 min(std::numeric_limits<int>::max());
	glm::ivec3 max(std::numeric_limits<int>::min());

	for (VoxChunk &chunk : model.getChunks())
	{
		min = glm::min(min, glm::ivec3(chunk.offset_x, chunk.offset_y, chunk.offset_z));
		max = glm::max(max, glm::ivec3(chunk.offset_x + chunk.width, chunk.offset_y + chunk.height, chunk.offset_z + chunk.depth));		
	}

	model.setSize(glm::ivec3(max - min));
	
	return (true);
}