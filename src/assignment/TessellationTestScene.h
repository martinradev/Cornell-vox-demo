#pragma once

#include "Scene.h"

#include "3d/Mesh.hpp"

#include "GBuffer.h"

#include <memory>

namespace FW {
	class TessellationTestScene : public Scene {

	public:

		TessellationTestScene(const std::string & sceneShader, GLContext * ctx, unsigned width, unsigned height);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);
		void setMesh(MeshBase * mesh) {
			m_mesh.reset(mesh);
		}

		TessellationTestScene::~TessellationTestScene() {
			
		}

	private:

		void loadShaders(GLContext * ctx);
		void updateUniforms(Window & wnd, const CameraControls & camera);

		std::string m_sceneShader;
		std::string m_programName;
		
		std::unique_ptr<MeshBase> m_mesh;
		std::unique_ptr<GBuffer> m_gbuffer;

		// knobs
		Vec4f m_knobs[10];

	};
};