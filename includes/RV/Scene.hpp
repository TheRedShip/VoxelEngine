/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/23 18:30:18 by ycontre           #+#    #+#             */
/*   Updated: 2025/03/28 16:01:16 by ycontre          ###   ########.fr       */
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

struct GPUDebug
{
	int	enabled;
	int	mode;
	int	voxel_treshold;
	int	box_treshold;
};

struct GPUVisibleVoxel
{
	uint32_t	voxel_per_pixel[WIDTH * HEIGHT];
	uint32_t	visible_voxel_index[WIDTH * HEIGHT];
	uint32_t	visible_voxel_flags[(WIDTH * HEIGHT) / 32];
	uint32_t	visible_voxel_count;
};

struct GPUVoxel;
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
		GPUVisibleVoxel					*getVisibleVoxels(void);
		GPUMaterial						getMaterial(int material_index);

		std::vector<FlatSVONode> flatNodes;
		std::vector<GPUVoxel> flatVoxels;
		
	private:
		GPUDebug					_gpu_debug;

		GPUVisibleVoxel				*_gpu_visible_voxels;
		std::vector<GPUMaterial>	_gpu_materials;


		Camera						*_camera;
};

#endif
