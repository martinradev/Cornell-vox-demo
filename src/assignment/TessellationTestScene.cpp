#include "TessellationTestScene.h"

#include "MeshRenderHelper.h"

#include "Globals.h"

namespace FW {

	TessellationTestScene::TessellationTestScene(const std::string & sceneShader, GLContext * ctx, unsigned width, unsigned height) :
	
		m_sceneShader(sceneShader),
		m_programName(sceneShader),
		m_mesh(nullptr)
	{
		loadShaders(ctx);

		m_gbuffer.reset(new GBuffer(width, height));

	}

	void TessellationTestScene::render(Window & wnd, const CameraControls & camera) {

		updateUniforms(wnd, camera);

		Vec2i sz = wnd.getSize();

		GLuint fbo = m_gbuffer->getFBO();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		glEnable(GL_DEPTH_TEST);
		glClearColor(0.5, 0.3, 0.4, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GLContext * gl = wnd.getGL();

		GLContext::Program * prog = gl->getProgram(m_programName.c_str());
		prog->use();

		glBindFragDataLocation(prog->getHandle(), 0, "diffuse");
		glBindFragDataLocation(prog->getHandle(), 1, "normal");
		glBindFragDataLocation(prog->getHandle(), 2, "position");

		if (m_mesh) {
			renderMeshTessellation(gl, m_mesh.get(), prog, 3);
		}
		

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


		// show stuff

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, sz.x, sz.y, 0, 0, sz.x *0.5f, sz.y * 0.5f, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, sz.x, sz.y, sz.x * 0.5f, 0, sz.x, sz.y * 0.5f, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glBlitFramebuffer(0, 0, sz.x, sz.y, sz.x * 0.5f, sz.y * 0.5f, sz.x, sz.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		//glReadBuffer(GL_DEPTH_ATTACHMENT);
		//glBlitFramebuffer(0, 0, sz.x, sz.y, 0, sz.y * 0.5f, 0.5f * sz.x, sz.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}

	void TessellationTestScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		default:
			break;
		}
	}

	void TessellationTestScene::activate(Window & wnd, CommonControls & controls) {
		updateGUI(wnd, controls);
	}
	void TessellationTestScene::updateGUI(Window & wnd, CommonControls & controls) {
		cleanUpGUI(wnd, controls);

		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(-40.0f), Vec4f(40.0f)), // knob1
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob2
			std::make_pair(Vec4f(-1.0f), Vec4f(1.0f)), // knob3
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob4
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob5
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob6
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob7
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob8
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob9
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)) // knob10
		};


		String xStr = sprintf("Knob%u.x = %%f", activeKnob + 1);
		String yStr = sprintf("Knob%u.y = %%f", activeKnob + 1);
		String zStr = sprintf("Knob%u.z = %%f", activeKnob + 1);
		String wStr = sprintf("Knob%u.w = %%f", activeKnob + 1);

		controls.beginSliderStack();
		controls.addSlider(&m_knobs[activeKnob].x, KNOB_SLIDE_DATA[activeKnob].first.x, KNOB_SLIDE_DATA[activeKnob].second.x, false, FW_KEY_NONE, FW_KEY_NONE, xStr);
		controls.addSlider(&m_knobs[activeKnob].y, KNOB_SLIDE_DATA[activeKnob].first.y, KNOB_SLIDE_DATA[activeKnob].second.y, false, FW_KEY_NONE, FW_KEY_NONE, yStr);
		controls.addSlider(&m_knobs[activeKnob].z, KNOB_SLIDE_DATA[activeKnob].first.z, KNOB_SLIDE_DATA[activeKnob].second.z, false, FW_KEY_NONE, FW_KEY_NONE, zStr);
		controls.addSlider(&m_knobs[activeKnob].w, KNOB_SLIDE_DATA[activeKnob].first.w, KNOB_SLIDE_DATA[activeKnob].second.w, false, FW_KEY_NONE, FW_KEY_NONE, wStr);
		controls.endSliderStack();
	}
	void TessellationTestScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}
	}

	void TessellationTestScene::loadShaders(GLContext * ctx) {
		const char vertexShader[] = "shaders/tessellation/simple_vertex.glsl";
		const char tessControlShader[] = "shaders/tessellation/simple_tess_control.glsl";
		const char tessEvalShader[] = "shaders/tessellation/simple_tess_eval.glsl";
		const char fragmentShader[] = "shaders/tessellation/simple_fragment.glsl";

		const char declShader[] = "shaders/raymarch/decl.glsl";
		const char noiseShader[] = "shaders/raymarch/noise.glsl"; // noise utilities
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms

		loadShader(ctx, { vertexShader }, { tessControlShader },
		{
			declShader,
			noiseShader,
			knobsShader,
			tessEvalShader
		},
		{ fragmentShader }, m_programName);
		ctx->checkErrors();
	}

	void TessellationTestScene::updateUniforms(Window & wnd, const CameraControls & camera) {
		GLContext * ctx = wnd.getGL();

		// update ray march program uniforms
		GLContext::Program * prog = ctx->getProgram(m_programName.c_str());

		prog->use();

		Mat4f toCamera = camera.getWorldToCamera();
		Mat4f toScreen = camera.getWorldToClip();

		ctx->setUniform(prog->getUniformLoc("toCamera"), toCamera);
		ctx->setUniform(prog->getUniformLoc("toScreen"), toScreen);
		ctx->setUniform(prog->getUniformLoc("normalToCamera"), toCamera.inverted().transposed());

		ctx->setUniform(prog->getUniformLoc("knob1"), m_knobs[0]);
		ctx->setUniform(prog->getUniformLoc("knob2"), m_knobs[1]);
		ctx->setUniform(prog->getUniformLoc("knob3"), m_knobs[2]);
		ctx->setUniform(prog->getUniformLoc("knob4"), m_knobs[3]);
		ctx->setUniform(prog->getUniformLoc("knob5"), m_knobs[4]);
		ctx->setUniform(prog->getUniformLoc("knob6"), m_knobs[5]);
		ctx->setUniform(prog->getUniformLoc("knob7"), m_knobs[6]);
		ctx->setUniform(prog->getUniformLoc("knob8"), m_knobs[7]);
		ctx->setUniform(prog->getUniformLoc("knob9"), m_knobs[8]);
		ctx->setUniform(prog->getUniformLoc("knob10"), m_knobs[9]);

		float currentTime = GLOBAL_TIMER.getElapsed();
		ctx->setUniform(prog->getUniformLoc("time"),currentTime);
	}

};