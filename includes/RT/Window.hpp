/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ycontre <ycontre@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/13 16:15:41 by TheRed            #+#    #+#             */
/*   Updated: 2024/12/23 18:35:35 by ycontre          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RT_WINDOW__HPP
# define RT_WINDOW__HPP

# include "RT.hpp"

class Scene;

class Window
{
	public:
		Window(Scene *scene, int width, int height, const char *title, int sleep);
		~Window(void);

		void		display();
		void		pollEvents();
		bool		shouldClose();
		
		void		process_input();
		
		static void	keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
		static void	mouseMoveCallback(GLFWwindow *window, double xpos, double ypos);
		static void	mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

		GLFWwindow	*getWindow(void) const;
		float		getFps(void) const;
		int			getFrameCount(void) const;

	private:
		GLFWwindow	*_window;
		Scene		*_scene;
		
		float		_fps;
		float		_delta;
		int			_frameCount;

};

#endif