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

# include "RV.hpp"

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

struct GPUVoxel
{
	alignas(16) glm::vec3 normal;
	alignas(16)	glm::ivec3 position;
	int color;
	int light;
};

struct GPUDebug
{
	int	enabled;
	int	mode;
	int	triangle_treshold;
	int	box_treshold;
};

struct FlatSVONode;

class Camera;
class VoxModel;

class Scene
{
	public:
		Scene();
		~Scene();

		void							parseScene(std::string &name);
		void							placeModel(VoxModel &model, glm::ivec3 position, std::vector<GPUVoxel> &voxel_data);

		void							addMaterial(GPUMaterial material);
		
		std::vector<GPUMaterial>		&getMaterialData();
		GPUDebug						&getDebug(void);

		Camera							*getCamera(void) const;
		GPUMaterial						getMaterial(int material_index);

		std::vector<FlatSVONode> flatNodes;
		std::vector<GPUVoxel> flatVoxels;
		
	private:

		std::vector<GPUMaterial>	_gpu_materials;

		GPUDebug					_gpu_debug;

		Camera						*_camera;
};

#endif
