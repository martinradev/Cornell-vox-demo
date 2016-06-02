#pragma once

#include "base/Math.hpp"
#include "gpu/GLContext.hpp"
#include "GPUBvh.h"
#include "gui/Window.hpp"
#include "3d/CameraControls.hpp"

namespace FW {

	class SnakeTrail {

	public:

		SnakeTrail(GLContext * gl, GPUBvh_Buffers & bvhBuffer, const Vec3f & origin, const Vec3f & normal);
		void update(GLContext * gl);
		void render(GLContext * gl, Window & wnd, CameraControls & camera);

	private:

		GPUBvh_Buffers m_bvhBuffer;
		Vec3f m_origin;
		Vec3f m_normal;
		GLuint m_buffer;

		std::string m_programCSName;
		std::string m_programRenderName;
	};

};