/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RT.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/27 14:52:10 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/19 16:51:28 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT__HPP
# define RT__HPP

# define WIDTH 1920
# define HEIGHT 1080

# define VOXEL_DIM 512
# define VOXEL_SIZE 0.1f

#define GLM_ENABLE_EXPERIMENTAL

# include "glm/glm.hpp"
# include "glm/gtx/string_cast.hpp"
# include "glm/gtc/matrix_transform.hpp"
# include "glm/gtc/type_ptr.hpp"
# include "glm/gtx/euler_angles.hpp"

# include "glad/gl.h"
# include "GLFW/glfw3.h"

# include "imgui/imgui.h"
# include "imgui/imgui_impl_glfw.h"
# include "imgui/imgui_impl_opengl3.h"

# include <queue>

# include <filesystem>
# include <algorithm>
# include <string.h>
# include <iostream>
# include <iomanip>
# include <fstream>
# include <sstream>
# include <bitset>
# include <chrono>
# include <vector>
# include <string>
# include <memory>
# include <set>
# include <map>

struct Vertex {
    glm::vec2 position;
    glm::vec2 texCoord;
};

# include "VoxModel.hpp"
# include "SVO.hpp"
# include "Buffer.hpp"
# include "Camera.hpp"
# include "Window.hpp"
# include "Shader.hpp"
# include "ShaderProgram.hpp"
# include "Scene.hpp"



#endif
