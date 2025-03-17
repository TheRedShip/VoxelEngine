/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:30:18 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/17 11:54:47 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_SCENE__HPP
# define RT_SCENE__HPP

# include "RT.hpp"

struct GPUMaterial
{
	alignas(16)	glm::vec3	color;
	float					emission;
	float					roughness;
	float					metallic;
	float					refraction;
	int						type;
	int						texture_index;
	int						emission_texture_index;
};

typedef struct s_Material
{
	glm::vec3	color;
	float		emission;
	float		roughness;
	float		metallic;
	float		refraction;
	int			type;
	int			texture_index;
	int			emission_texture_index;
}				Material;

class Camera;

class Scene
{
	public:
		Scene();
		~Scene();

		void							parseScene(std::string &name);

		void							addMaterial(Material *material);
		
		std::vector<GPUMaterial>		&getMaterialData();

		Camera							*getCamera(void) const;
		GPUMaterial						getMaterial(int material_index);
		
	private:

		std::vector<GPUMaterial>	_gpu_materials;

		Camera						*_camera;
};

#endif
