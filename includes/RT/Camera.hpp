/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 13:59:57 by TheRed            #+#    #+#             */
/*   Updated: 2025/03/17 12:03:19 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_CAMERA__HPP
# define RT_CAMERA__HPP

# include "RT.hpp"

struct GPUCamera
{
	glm::mat4				view_matrix;
    alignas(16) glm::vec3	camera_position;
	float					aperture_size;
	float					focus_distance;
	float					fov;

	int						bounce;
};

class Camera
{
	public:

		Camera(glm::vec3 startPos, glm::vec3 startUp, float startYaw, float startPitch);
		~Camera(void);

		void		update(float deltaTime);
		void		processMouse(float xoffset, float yoffset, bool constrainPitch);
		void		processKeyboard(bool forward, bool backward, bool left, bool right, bool up, bool down);
		
		void		updateCameraVectors();
		void		updateCameraDirections();

		glm::vec3	getPosition();
		glm::vec2	getDirection();
		glm::mat4	getViewMatrix();
		
		float		getVelocity();

		float		&getFov();
		float		&getAperture();
		float		&getFocus();
		int			&getBounce();
		
		GPUCamera	getGPUData();

		void		setPosition(glm::vec3 position);
		void		setDirection(float pitch, float yaw);
		void		setDOV(float aperture, float focus);
		void		setBounce(int b);
		void		setFov(float fov);

	private:

		glm::vec3	_position;
		glm::vec3	_forward;
		glm::vec3	_up;
		glm::vec3	_right;

		float		_pitch;
		float		_yaw;

		glm::vec3	_velocity;
    	glm::vec3	_acceleration;

		float _maxSpeed = 8.0f;
		float _acceleration_rate = 40.0f;
		float _deceleration_rate = 10000.0f;
		float _sensitivity = 0.2f;

		float _aperture_size = 0.0f;
		float _focus_distance = 1.0f;
		float _fov = 90.0f;

		int	_bounce = 5;
};

#endif
