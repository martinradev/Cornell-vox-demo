#include "SnakeTrail.h"
#include "ShaderSetup.h"
#include "MemoryAllignedAllocator.h"
#include "Globals.h"
namespace FW {

	static struct _MM_ALIGN16 SnakeParticle {
		SnakeParticle() {}
		SnakeParticle(const Vec3f & position, const Vec3f & normal) :
			pos(position, 0),
			n(normal, 0) {}

		Vec4f pos;
		Vec4f n;
	};

	SnakeTrail::SnakeTrail(GLContext * gl, GPUBvh_Buffers & bvhBuffer, const Vec3f & origin, const Vec3f & normal)
		:
		m_bvhBuffer(bvhBuffer),
		m_origin(origin),
		m_normal(normal),
		m_programCSName("snake_cs"),
		m_programRenderName("snake_draw")
	{

		std::vector<SnakeParticle, aligned_allocator<SnakeParticle, 16> > particleData(1);
		particleData[0] = SnakeParticle(origin, normal);

		glGenBuffers(1, &m_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SnakeParticle), particleData.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		const char lightCS[] = "shaders/snake/move_particles.glsl";
		const char bvhCS[] = "shaders/bvh/bvh.glsl";
		const char declShader[] = "shaders/raymarch/decl.glsl";
		const char noiseShader[] = "shaders/raymarch/noise.glsl";
		const char knobsShader[] = "shaders/raymarch/knobs.glsl";

		const char simpleVertShader[] = "shaders/snake/vert.glsl";
		const char simpleFragShader[] = "shaders/snake/frag.glsl";

		loadShader(gl,
		{ declShader, knobsShader, noiseShader, bvhCS, lightCS }, m_programCSName);

		loadShader(gl, simpleVertShader, simpleFragShader, m_programRenderName);

		gl->checkErrors();

	}
	void SnakeTrail::update(GLContext * gl) {

		GLContext::Program * prog = gl->getProgram(m_programCSName.c_str());
		prog->use();

		gl->setUniform(prog->getUniformLoc("time"), GLOBAL_TIMER.getElapsed());

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_bvhBuffer.m_triangles_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_bvhBuffer.m_indices_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bvhBuffer.m_nodes_ssbo);
		int m_numParticles = 1;
		static const int localGroupSize = 64;
		unsigned numGroups = (m_numParticles + localGroupSize - 1) / localGroupSize;
		glDispatchCompute(numGroups, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, 0);
	}

	void SnakeTrail::render(GLContext * gl, Window & wnd, CameraControls & camera) {

		GLContext::Program * prog = gl->getProgram(m_programRenderName.c_str());
		prog->use();

		gl->setUniform(prog->getUniformLoc("posToClip"), camera.getWorldToClip());
		glEnable(GL_POINT_SMOOTH);
		glPointSize(6);

		glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SnakeParticle), 0);

		glDrawArrays(GL_POINTS, 0, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

};