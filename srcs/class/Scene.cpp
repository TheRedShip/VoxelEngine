/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:29:41 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/17 12:22:41 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scene.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Scene::Scene(std::string &name)
{
	// std::ifstream	file(name);
	(void) name;
	_camera = new Camera(glm::vec3(30.0f, 15.0f, 15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f);
	
	// if (!file.is_open())
	// {
	// 	std::cerr << "Can't open scene file" << std::endl;
	// 	return ;
	// }

	std::cout << "Parsing done" << std::endl;
}

Scene::~Scene()
{
	delete (_camera);
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
