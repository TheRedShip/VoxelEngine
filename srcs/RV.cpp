/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RV.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/30 16:56:56 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RV.hpp"

void					setupScreenTriangle(GLuint *VAO);
void					drawScreenTriangle(GLuint VAO, GLuint output_texture, GLuint program);

std::vector<GLuint>		generateTextures(unsigned int textures_count);

std::vector<Buffer *>	createDataOnGPU(Scene &scene);
void					updateDataOnGPU(Scene &scene, std::vector<Buffer *> buffers);

int main(int argc, char **argv)
{
	std::string args = "";
	if (argc == 2)
		args = argv[1];

	Scene		scene;
	Window		window(&scene, WIDTH, HEIGHT, "RedVoxel", 0);
	
	scene.parseScene(args);

	GLuint VAO;
	setupScreenTriangle(&VAO);

	std::vector<GLuint> textures = generateTextures(1);
	
	ShaderProgram visible_voxel_tracing_program;
	Shader visible_voxel = Shader(GL_COMPUTE_SHADER, "shaders/visible_voxel.glsl");

	visible_voxel_tracing_program.attachShader(&visible_voxel);
	visible_voxel_tracing_program.link();

	ShaderProgram raytracing_program;
	Shader raytracing = Shader(GL_COMPUTE_SHADER, "shaders/raytracing.glsl");

	raytracing_program.attachShader(&raytracing);
	raytracing_program.link();

	ShaderProgram output_program;
	Shader output = Shader(GL_COMPUTE_SHADER, "shaders/output.glsl");

	output_program.attachShader(&output);
	output_program.link();

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

		//first pass: visible voxel tracing

		// resetting amount of voxels hit
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[4]->getID());
		glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R8UI, GL_RED, GL_UNSIGNED_BYTE, NULL);

		visible_voxel_tracing_program.use();
		visible_voxel_tracing_program.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
		visible_voxel_tracing_program.set_int("u_frameCount", window.getFrameCount());
		visible_voxel_tracing_program.set_float("u_voxelSize", VOXEL_SIZE);
		visible_voxel_tracing_program.dispathCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
		
		// getting amount of voxels hit
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[4]->getID());
		uint32_t visible_count = 0;
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offsetof(GPUVisibleVoxel, visible_voxel_count), sizeof(uint32_t), &visible_count);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		//
		
		//second pass getting lighting information
		int workgroupSize = 128;
		int numWorkgroups = (visible_count + workgroupSize - 1) / workgroupSize;

		raytracing_program.use();
		raytracing_program.set_int("u_frameCount", window.getFrameCount());
		raytracing_program.set_int("u_voxelDim", VOXEL_DIM);
		raytracing_program.set_float("u_voxelSize", VOXEL_SIZE);
		raytracing_program.set_float("u_time", (float)(glfwGetTime()));
		raytracing_program.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
		raytracing_program.dispathCompute(numWorkgroups, 1, 1);

		//third pass: output to texture
		output_program.use();
		output_program.set_vec2("u_resolution", glm::vec2(WIDTH, HEIGHT));
		output_program.dispathCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);


		window.imGuiNewFrame();

		render_program.use();
		drawScreenTriangle(VAO, textures[window.getOutputTexture()], render_program.getProgram());

		window.imGuiRender(raytracing_program);

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
