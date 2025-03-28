/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 16:16:24 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/28 16:06:53 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Window.hpp"

void GLFWErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error (%d): %s\n", error, description);
}

Window::Window(Scene *scene, int width, int height, const char *title, int sleep)
{
	_scene = scene;
	_fps = 0;
	_frameCount = 0;
	_pixelisation = 0;
	_output_texture = 0;

	glfwSetErrorCallback(GLFWErrorCallback);
	if (!glfwInit())
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	_window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!_window)
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		exit(-1);
	}
	
	glfwMakeContextCurrent(_window);
	glfwSetWindowUserPointer(_window, this);

	glfwSetKeyCallback(_window, keyCallback);
	glfwSetCursorPosCallback(_window, mouseMoveCallback);
	glfwSetMouseButtonCallback(_window, mouseButtonCallback);

	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(sleep);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

Window::~Window(void)
{
	glfwTerminate();
}


void Window::process_input()
{
	bool forward = glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS;
	bool backward = glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS;
	bool left = glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS;
	bool right = glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS;
	bool up = glfwGetKey(_window, GLFW_KEY_SPACE) == GLFW_PRESS;
	bool down = glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

	if (forward || backward || left || right || up || down)
		_frameCount = 0;

	_scene->getCamera()->processKeyboard(forward, backward, left, right, up, down);
}

void Window::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	(void) win; (void) xpos; (void) ypos;

	static double lastX = 0;
	static double lastY = 0;

	if (lastX == 0 && lastY == 0)
	{
		lastX = xpos;
		lastY = ypos;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) 
	{
		win->_scene->getCamera()->processMouse(xoffset, yoffset, true);
		win->_frameCount = 0;
	}

	lastX = xpos;
	lastY = ypos;
}
void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    (void) win; (void) button; (void) mods;
	
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		win->_frameCount = 0;
}
void Window::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    (void) win; (void) key; (void) scancode; (void) action; (void) mods;
	
	if (key == 67 && action == GLFW_PRESS)
	{
		glm::vec3 pos = win->_scene->getCamera()->getPosition();
		glm::vec2 dir = win->_scene->getCamera()->getDirection();
		float aperture = win->_scene->getCamera()->getAperture();
		float focus = win->_scene->getCamera()->getFocus();
		float fov = win->_scene->getCamera()->getFov();
		int	bounce = win->_scene->getCamera()->getBounce();

		std::cout << "\nCAM\t" << pos.x << " " << pos.y << " " << pos.z << "\t"
				<< dir.x << " " << dir.y << " " << "\t"
				<< aperture << " " << focus << " " << fov << "\t" << bounce
				<< std::endl;
	}
}

void Window::updateDeltaTime()
{
	static double	lastTime = glfwGetTime();
	double			currentTime = glfwGetTime();
	
	_delta = currentTime - lastTime;

	lastTime = currentTime;
	_fps = 1.0f / _delta;
}

void Window::display()
{
	if (accumulate)
		_frameCount++;

	if (_scene->getCamera()->getVelocity() > 0.0f)
		_frameCount = 0;

    glfwSwapBuffers(_window);
}

void Window::pollEvents()
{
	this->process_input();
	_scene->getCamera()->update(_delta);
	
    glfwPollEvents();
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(_window);
}

void Window::imGuiNewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Window::imGuiRender(ShaderProgram &raytracing_program)
{
	bool has_changed = false;
	
	ImGui::Begin("Settings");

	ImGui::Text("Fps: %d", int(_fps));
	ImGui::Text("Frame: %d", _frameCount);
	ImGui::SliderInt("Output texture", &_output_texture, 0, 7);
	
	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Camera"))
	{

		if (ImGui::Checkbox("Accumulate", &accumulate))
			_frameCount = 0;

		has_changed |= ImGui::SliderInt("Bounce", &_scene->getCamera()->getBounce(), 0, 20);
		has_changed |= ImGui::SliderFloat("FOV", &_scene->getCamera()->getFov(), 1.0f, 180.0f);
		has_changed |= ImGui::SliderFloat("Aperture", &_scene->getCamera()->getAperture(), 0.0f, 1.0f);
		has_changed |= ImGui::SliderFloat("Focus", &_scene->getCamera()->getFocus(), 0.0f, 150.0f);
	}


	if (ImGui::CollapsingHeader("Material"))
	{

		ImGui::BeginChild("Header", ImVec2(0, 400), true, 0);

		for (unsigned int i = 0; i < _scene->getMaterialData().size(); i++)
		{
			GPUMaterial &mat = _scene->getMaterialData()[i];

			ImGui::PushID(i);
			
			ImGui::Text("Material %d", i);
			has_changed |= ImGui::ColorEdit3("Color", &mat.color[0]);
			has_changed |= ImGui::SliderFloat("Emission", &mat.emission, 0.0f, 10.0f);
			
			if (mat.type == 0)
			{
				has_changed |= ImGui::SliderFloat("Roughness", &mat.roughness, 0.0f, 1.0f);
				has_changed |= ImGui::SliderFloat("Metallic", &mat.metallic, 0.0f, 1.0f);
			}
			else if (mat.type == 1)
				has_changed |= ImGui::SliderFloat("Refraction", &mat.refraction, 1.0f, 5.0f);
			else if (mat.type == 2)
			{
				has_changed |= ImGui::SliderFloat("Transparency", &mat.roughness, 0.0f, 1.0f);
				has_changed |= ImGui::SliderFloat("Refraction", &mat.refraction, 1.0f, 2.0f);
				has_changed |= ImGui::SliderFloat("Proba", &mat.metallic, 0., 1.);
			}
			else if (mat.type == 3)
			{
				has_changed |= ImGui::SliderFloat("Checker Scale", &mat.refraction, 0.0f, 40.0f);
				has_changed |= ImGui::SliderFloat("Roughness", &mat.roughness, 0.0f, 1.0f);
				has_changed |= ImGui::SliderFloat("Metallic", &mat.metallic, 0.0f, 1.0f);
			}
			has_changed |= ImGui::SliderInt("Type", &mat.type, 0, 3);

			ImGui::PopID();

			ImGui::Separator();
		}
		ImGui::EndChild();

	}

	if (ImGui::CollapsingHeader("Debug"))
	{
		if (ImGui::Checkbox("Enable", (bool *)(&_scene->getDebug().enabled)))
		{
			raytracing_program.setDefine("DEBUG", std::to_string(_scene->getDebug().enabled));
			raytracing_program.reloadShaders();
			has_changed = true;
		}
		ImGui::Separator();
		has_changed |= ImGui::SliderInt("Debug mode", &_scene->getDebug().mode, 0, 2);
		has_changed |= ImGui::SliderInt("Box treshold", &_scene->getDebug().box_treshold, 1, 2000);
		has_changed |= ImGui::SliderInt("Voxel treshold", &_scene->getDebug().voxel_treshold, 1, 2000);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (has_changed)
		_frameCount = (accumulate == 0) - 1;
}

GLFWwindow	*Window::getWindow(void) const
{
	return (_window);
}

float		Window::getFps(void) const
{
	return (_fps);
}

int			Window::getFrameCount(void) const
{
	return (_frameCount);
}

void		Window::setFrameCount(int nb)
{
	_frameCount = nb;
}

bool		&Window::getAccumulate(void)
{
	return (accumulate);
}

int			Window::getOutputTexture(void) const
{
	return (_output_texture);
}
