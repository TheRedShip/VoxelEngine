/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:51:49 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/17 16:58:09 by ycontre          ###   ########.fr       */
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
		raytracing_program.set_int("u_voxelDim", VOXEL_DIM);
		raytracing_program.set_float("u_voxelSize", VOXEL_SIZE);
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
