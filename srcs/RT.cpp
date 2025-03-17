/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/17 12:10:31 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void					setupScreenTriangle(GLuint *VAO);
void					drawScreenTriangle(GLuint VAO, GLuint output_texture, GLuint program);

std::vector<GLuint>		generateTextures(unsigned int textures_count);

std::vector<Buffer *>	createDataOnGPU(Scene &scene);
void					updateDataOnGPU(Scene &scene, std::vector<Buffer *> buffers);

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;
	
	std::string args = "";
	Scene		scene(args);
	
	Window		window(&scene, WIDTH, HEIGHT, "RT_GPU", 0);

	GLuint VAO;
	setupScreenTriangle(&VAO);

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
					voxelData[indexData + 0] = 150 + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 10; // Red
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
					voxelData[indexData + 1] = 150 + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 10;
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
	glActiveTexture(GL_TEXTURE1);  // Bind to texture unit 2
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
	glActiveTexture(GL_TEXTURE2);  // Bind to texture unit 2
	glBindTexture(GL_TEXTURE_3D, voxelNormalTex);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, dim, dim, dim, 0, GL_RGB, GL_FLOAT, voxelNormals.data());
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	std::vector<GLuint> textures = generateTextures(1);
	
	ShaderProgram raytracing_program;
	Shader compute = Shader(GL_COMPUTE_SHADER, "shaders/compute.glsl");

	raytracing_program.attachShader(&compute);
	raytracing_program.link();

	ShaderProgram render_program;
	Shader vertex = Shader(GL_VERTEX_SHADER, "shaders/vertex.vert");
	Shader frag = Shader(GL_FRAGMENT_SHADER, "shaders/frag.frag");
	render_program.attachShader(&vertex);
	render_program.attachShader(&frag);
	render_program.link();

	std::vector<Buffer *> buffers = createDataOnGPU(scene);

	while (!window.shouldClose())
	{
		window.updateDeltaTime();
		
		updateDataOnGPU(scene, buffers);
		
		glClear(GL_COLOR_BUFFER_BIT);
		
		raytracing_program.use();
		raytracing_program.set_int("u_frameCount", window.getFrameCount());
		raytracing_program.set_float("u_time", (float)(glfwGetTime()));
		raytracing_program.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
		
		raytracing_program.dispathCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);

		window.imGuiNewFrame();

		render_program.use();
		drawScreenTriangle(VAO, textures[window.getOutputTexture()], render_program.getProgram());

		window.imGuiRender();

		window.display();
		window.pollEvents();

		// glClearTexImage(textures[3], 0, GL_RGBA, GL_FLOAT, nullptr);
		// glClearTexImage(textures[4], 0, GL_RGBA, GL_FLOAT, nullptr);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return (0);
}
