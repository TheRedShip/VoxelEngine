/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT_utils.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 23:21:09 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/17 11:54:25 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RV.hpp"

void				setupScreenTriangle(GLuint *VAO)
{
	GLuint VBO;

	Vertex vertices[3] = {{{-1.0f, -1.0f}, {0.0f, 0.0f}},{{3.0f, -1.0f}, {2.0f, 0.0f}},{{-1.0f, 3.0f}, {0.0f, 2.0f}}};
	size_t size = sizeof(vertices) / sizeof(Vertex) / 3; // size 1

    glGenVertexArrays(1, VAO);
    glBindVertexArray(*VAO);
	
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void				drawScreenTriangle(GLuint VAO, GLuint output_texture, GLuint program)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, output_texture);
	glUniform1i(glGetUniformLocation(program, "screenTexture"), 0);
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 1 * 3); // size 1
}

//0 output
std::vector<GLuint> generateTextures(unsigned int textures_count)
{
	std::vector<GLuint> textures(textures_count);

	glGenTextures(textures_count, textures.data());
	for (unsigned int i = 0; i < textures_count; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindImageTexture(i, textures[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	}
	return (textures);
}

std::vector<Buffer *>	createDataOnGPU(Scene &scene)
{
	GLint max_gpu_size;
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &max_gpu_size);

	const std::vector<FlatSVONode> &flatNodes = scene.flatNodes;
	const std::vector<GPUVoxel> &flatVoxels = scene.flatVoxels;

	std::vector<Buffer *> buffers;
	
	buffers.push_back(new Buffer(Buffer::Type::UBO, 0, sizeof(GPUCamera), nullptr));
	buffers.push_back(new Buffer(Buffer::Type::UBO, 1, sizeof(GPUDebug), nullptr));
	
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 0, sizeof(FlatSVONode) * flatNodes.size(), flatNodes.data()));
	buffers.push_back(new Buffer(Buffer::Type::SSBO, 1, sizeof(GPUVoxel) * flatVoxels.size(), flatVoxels.data()));

	return (buffers);
}

void	updateDataOnGPU(Scene &scene, std::vector<Buffer *> buffers)
{

	GPUCamera camera_data = scene.getCamera()->getGPUData();
	buffers[0]->update(&camera_data, sizeof(GPUCamera));

	buffers[1]->update(&scene.getDebug(), sizeof(GPUDebug));
}
