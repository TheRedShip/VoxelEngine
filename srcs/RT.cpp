/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2024/12/23 18:38:38 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

int main(int argc, char **argv)
{
	Scene		scene;

	Window		window(&scene, WIDTH, HEIGHT, "RT_GPU", 0);
	Shader		shader("shaders/vertex.vert", "shaders/frag.frag", "shaders/compute.glsl");

	shader.attach();

	Vertex vertices[3] = {{{-1.0f, -1.0f}, {0.0f, 0.0f}},{{3.0f, -1.0f}, {2.0f, 0.0f}},{{-1.0f, 3.0f}, {0.0f, 2.0f}}};
	size_t size = sizeof(vertices) / sizeof(Vertex) / 3;
	shader.setupVertexBuffer(vertices, size);

	const int dim = 128;
	std::vector<unsigned char> voxelData(dim * dim * dim * 4, 0);  // 4 channels per voxel
	std::vector<float> voxelNormals(dim * dim * dim * 3, 0.0f);  // 4 channels per voxel

	memset(voxelData.data(), 0, voxelData.size());
	// Fill a sphere of voxels with a red color (R=255, G=0, B=0, A=255 indicates solid)
	for (int z = 0; z < dim; ++z) {
		for (int y = 0; y < dim; ++y) {
			for (int x = 0; x < dim; ++x) {
				float dx = x - dim / 2.0f;
				float dy = (y + 30) - dim / 2.0f;
				float dz = z - dim / 2.0f;
				int indexData = 4 * (x + dim * (y + dim * z));
				int indexNormals = 3 * (x + dim * (y + dim * z));
				if (std::sqrt(dx * dx + dy * dy + dz * dz) < 16.0 / 2.0f) {
					voxelData[indexData + 0] = 150 + (float(rand()) / RAND_MAX) * 10; // Red
					voxelData[indexData + 1] = 0;   // Green
					voxelData[indexData + 2] = 0;   // Blue
					voxelData[indexData + 3] = 255; // Alpha (non-zero means solid)

					voxelNormals[indexNormals + 0] = dx / 16.0f;
					voxelNormals[indexNormals + 1] = dy / 16.0f;
					voxelNormals[indexNormals + 2] = dz / 16.0f;
				}
				if (y == 0)
				{
					voxelData[indexData + 0] = 0; // Red
					voxelData[indexData + 1] = 150 + (float(rand()) / RAND_MAX)  * 10;
					voxelData[indexData + 2] = 0;   // Blue
					voxelData[indexData + 3] = 255; // Alpha (non-zero means solid)

					voxelNormals[indexNormals + 0] = 0;
					voxelNormals[indexNormals + 1] = 1;
					voxelNormals[indexNormals + 2] = 0;
				}
			}
		}
	}


	// Create and upload the 3D texture at texture unit index 2
	GLuint voxelTex;
	glGenTextures(1, &voxelTex);
	glActiveTexture(GL_TEXTURE2);  // Bind to texture unit 2
	glBindTexture(GL_TEXTURE_3D, voxelTex);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, dim, dim, dim, 0, GL_RGBA, GL_UNSIGNED_BYTE, voxelData.data());
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Create and upload the 3D texture at texture unit index 2
	GLuint voxelNormalTex;
	glGenTextures(1, &voxelNormalTex);
	glActiveTexture(GL_TEXTURE3);  // Bind to texture unit 2
	glBindTexture(GL_TEXTURE_3D, voxelNormalTex);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, dim, dim, dim, 0, GL_RGB, GL_FLOAT, voxelNormals.data());
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	while (!window.shouldClose())
	{
		glUseProgram(shader.getProgramCompute());

		shader.set_int("u_frameCount", window.getFrameCount());
		shader.set_float("u_time", (float)(glfwGetTime()));
		shader.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
		shader.set_vec3("u_cameraPosition", scene.getCamera()->getPosition());
		shader.set_mat4("u_viewMatrix", scene.getCamera()->getViewMatrix());
		
		glDispatchCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader.getProgram());
		shader.drawTriangles(size);

		std::cout << "\rFrame: " << window.getFrameCount() << " Fps: " << int(window.getFps()) << "                        " << std::flush;
		
		window.display();
		window.pollEvents();		
	}
	
	return (0);
}