
#include "App.hpp"

#include "RayMarchScene.h"
#include "HeightMapScene.h"
#include "BrightPassFilter.h"
#include "BloomFilter.h"
#include "ToneMapper.h"
#include "base/Main.hpp"
#include "gpu/GLContext.hpp"
#include "3d/Mesh.hpp"
#include "3d/Texture.hpp"
#include "io/File.hpp"
#include "io/StateDump.hpp"
#include "base/Random.hpp"
#include "ShaderSetup.h"
#include "TessellationTestScene.h"
#include "ForwardShading.h"
#include "LightParticles.h"
#include "TexturePool.h"
#include "GlobalSyncVars.h"
#include "GPUPrefixScan.h"
#include "Util.h"
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <fstream>
#include <chrono>
#include <omp.h>
using namespace FW;

// extern to be used from all scenes
unsigned activeKnob;
bool updateGUIExt;
FW::Action actionExt = FW::Action::Action_None; // scene action

namespace FW {
	FW::Timer GLOBAL_TIMER;
}
namespace FWSync {
	// cloth effect
	 const sync_track * CLOTH_WIND_FORCE;
	 const sync_track * CLOTH_Y_POS;

	// ball pulse effect
	 const sync_track * BALL_LIGHT_INTENSITY_R;
	 const sync_track * BALL_LIGHT_INTENSITY_G;
	 const sync_track * BALL_LIGHT_INTENSITY_B;

	// ball displace effect
	 const sync_track * BALL_DISP_1;
	 const sync_track * BALL_DISP_2;
	 const sync_track * BALL_DISP_3;
	 const sync_track * BALL_DISP_4;

	 const sync_track * BALL_DISP_5;
	 const sync_track * BALL_DISP_6;
	 const sync_track * BALL_DISP_7;
	 const sync_track * BALL_DISP_8;

	 const sync_track * BALL_DISP_9;
	 const sync_track * BALL_DISP_10;
	 const sync_track * BALL_DISP_11;
	 const sync_track * BALL_DISP_12;

	 const sync_track * BALL_DISP_13;
	 const sync_track * BALL_DISP_14;
	 const sync_track * BALL_DISP_15;
	 const sync_track * BALL_DISP_16;

	// text offset
	 const sync_track * TEXT_OFFSET;
	 const sync_track * TEXT_EXPLODE;
	 const sync_track * TEXT_SUCK;
	 const sync_track * TEXT_RESTART;
	 const sync_track * TEXT_ON;

	 const sync_track * BOUNCE_TIME;
	 const sync_track * BOUNCE_PERIOD;
	 const sync_track * BOUNCE_SCALE;

	 const sync_track * DM_MESH_PARTICLE_SWITCH;
	 const sync_track * CAM_PATH_TIME;

	 const sync_track * BLUR_OUT;
}

namespace FW {
	TexturePool * TEXTURE_POOL;
};

//------------------------------------------------------------------------

App::App(void)
: m_commonCtrl(CommonControls::Feature_Default & ~CommonControls::Feature_RepaintOnF5),
m_cameraCtrl(&m_commonCtrl, CameraControls::Feature_Default | CameraControls::Feature_StereoControls),
m_activeKnob(Knob::Knob1),
m_prevKnob(Knob::Knob4),
m_action(Action::Action_None),
m_dynamicMesh(nullptr),
m_scene(nullptr),
m_followPath(true),
m_renderPath(false),
m_displaySceneTabs(false)
{
	
    m_commonCtrl.showFPS(true);
    m_commonCtrl.addStateObject(this);
    m_cameraCtrl.setKeepAligned(true);
	m_commonCtrl.showControls(false);
    m_window.addListener(&m_cameraCtrl);
	m_commonCtrl.addSeparator();
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_ReloadShaders,			FW_KEY_ENTER,   "Reload shaders (ENTER)");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_AddCameraPoint, FW_KEY_SPACE, "Add cam point (SPACE)");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_DeletePrevPoint, FW_KEY_DELETE, "Rem cam point (Del)");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_SavePath, FW_KEY_NONE, "Save cam points");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_RenderPath, FW_KEY_NONE, "Render path");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_FollowPath, FW_KEY_NONE, "Follow path");
	m_commonCtrl.addToggle(&m_displaySceneTabs, FW_KEY_Q, "Show scene tabs (Q)");
	m_commonCtrl.addSeparator();
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob1, FW_KEY_1, "Knob1");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob2, FW_KEY_2, "Knob2");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob3, FW_KEY_3, "Knob3");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob4, FW_KEY_4, "Knob4");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob5, FW_KEY_5, "Knob5");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob6, FW_KEY_6, "Knob6");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob7, FW_KEY_7, "Knob7");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob8, FW_KEY_8, "Knob8");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob9, FW_KEY_9, "Knob9");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob10, FW_KEY_0, "Knob10");
	activeKnob = 0;
	
	m_window.setTitle("Demo Project");
    m_window.addListener(this);
	m_window.setSize(Vec2i(800, 600));
	m_window.setFullScreen(true);
    m_window.addListener(&m_commonCtrl);
	
	ShowCursor(false);

	m_commonCtrl.setStateFilePrefix( "Cornell Vox by Varko" );
	
	GLContext * gl = m_window.getGL(); // grab the appropriate gl context to be able to setup()
	

	/* TODO */

	/*int numValues = 1024*1024;
	std::vector<int> values(numValues);
	int totalSum = 0;
	for (int i = 0; i < numValues; ++i) {
		values[i] = rand()%32;
		totalSum += values[i];
	}

	// compute prefix scan
	std::vector<int> prefix(numValues);
	prefix[0] = 0;
	for (int i = 1; i < numValues; ++i) {
		prefix[i] = prefix[i - 1] + values[i-1];
	}

	GLuint vBuffer;
	GLuint blockBuffer;
	glGenBuffers(1, &vBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * numValues, values.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	int numBlocks = (numValues+1023) / 1024;
	glGenBuffers(1, &blockBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1025, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	
	
	GPUPrefixScan::loadProgram(gl, "shaders/compute/prefix_scan.glsl", "shaders/compute/prefix_block_add.glsl");
	GLint elapsedTime;
	GLuint elapsedBuf;
	
	
	for (int i = 0; i < 10; ++i) {
		glGenQueries(1, &elapsedBuf);
		glBeginQuery(GL_TIME_ELAPSED, elapsedBuf);

		GPUPrefixScan::scan(gl, vBuffer, blockBuffer, numValues);
		
		glEndQuery(GL_TIME_ELAPSED);

		int stopTimerAvailable = 0;
		while (!stopTimerAvailable) {
			glGetQueryObjectiv(elapsedBuf,
				GL_QUERY_RESULT_AVAILABLE,
				&stopTimerAvailable);
		}

		glGetQueryObjectui64v(elapsedBuf, GL_QUERY_RESULT, &elapsedTime);
		printf("Time spent on the GPU: %lf ms\n", double(elapsedTime) / 1000000.0);
		
	}
	std::vector<int> prefixGPU(numValues);
	std::vector<int> blockValues(1025);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numValues * sizeof(int), prefixGPU.data());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockBuffer);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 1025 * sizeof(int), blockValues.data());
	
	exit(1);*/
	
	/* END TODO */

	m_cameraCtrl.setFar(200.0f);
	GLOBAL_TIMER = FW::Timer();
	GLOBAL_TIMER.start();

	TEXTURE_POOL = new TexturePool();

	setupRocket();
	setupScenes();

	// ready to start
	BASS_Start();
	BASS_ChannelPlay(m_stream, false);
}

//------------------------------------------------------------------------

App::~App(void)
{
	
}

//------------------------------------------------------------------------

bool App::handleEvent(const Window::Event& ev)
{
	if (ev.type == Window::EventType_Close)
	{

		
		m_window.showModalMessage("Exiting...");
		delete this;
		return true;
	}
	/*
	if (ev.type == Window::EventType_KeyUp) {

		if (ev.key == FW_KEY_MOUSE_LEFT && m_displaySceneTabs) {

			// get mouse pixel position and see what we clicked

			Vec2i pos = ev.mousePos;

			Vec2i wndSize = m_window.getSize();

			const unsigned int sceneBoxSide = wndSize.x / m_allScenes.size();
			const unsigned int height = 80;

			if (pos.y > wndSize.y - height) {

				// definitely clicked a scene

				if (m_scene != nullptr) {
					m_scene->cleanUpGUI(m_window, m_commonCtrl);
				}

				m_scene = m_allScenes[pos.x / sceneBoxSide].m_scene;
				m_scene->activate(m_window, m_commonCtrl);
			}

		}

	}

    Action action = actionExt;
	actionExt = Action::Action_None;

	if (action != Action::Action_None) {
		// handle action
		m_scene->handleAction(action, m_window, m_commonCtrl);
	}

	

	action = m_action;
	m_action = Action::Action_None;
	if (action != Action::Action_None) {
		// handle action
		m_scene->handleAction(action, m_window, m_commonCtrl);
	}

	if (action == Action::Action_ReloadShaders) {
		m_centerMesh->loadShaders(m_window.getGL());
		m_lightSystem->setupShaders(m_window.getGL());
		m_dmLightSystem->setupShaders(m_window.getGL());
		m_dynamicMesh->loadShaders(m_window.getGL());
	}

	if (action == Action::Action_AddCameraPoint) {
		m_cameraPath->addControState(m_cameraCtrl.getCameraToWorld());
	} else if (action == Action::Action_DeletePrevPoint) {
		m_cameraPath->popState();
	} else if (action == Action::Action_SavePath) {
		m_cameraPath->savePath("assets/cam_path.txt");
	}
	else if (action == Action::Action_FollowPath) {
		m_followPath = !m_followPath;
	}
	else if (action == Action::Action_RenderPath) {
		m_renderPath = !m_renderPath;
	}
	
	if (m_activeKnob != m_prevKnob) {
		activeKnob = (unsigned)m_activeKnob;
		m_prevKnob = m_activeKnob;
		if (m_scene != nullptr) updateGUIExt = true;
	}

	if (updateGUIExt) m_scene->updateGUI(m_window, m_commonCtrl);
	updateGUIExt = false;*/
    m_window.setVisible(true);

    if (ev.type == Window::EventType_Paint)
        renderFrame(m_window.getGL());
    m_window.repaint();
    return false;
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------

void App::waitKey(void)
{
    printf("Press any key to continue . . . ");
    _getch();
    printf("\n\n");
}

void App::renderFrame(GLContext* gl)
{
	// sync stuff first

	syncScenes();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.3, 0.4, 1.0);
	
	//glClearColor(rgb.x, rgb.y, rgb.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_scene) {

		m_scene->render(m_window, m_cameraCtrl);

	}
	
	if (m_displaySceneTabs) {
		renderSceneOptions(gl);
	}
	
	// debug cam path
	if (m_renderPath) {
		glUseProgram(0);
		glMatrixMode(GL_PROJECTION);
		Mat4f worldToCamera = m_cameraCtrl.getWorldToCamera();
		Mat4f projection = gl->xformFitToView(Vec2f(-1.0f, -1.0f), Vec2f(2.0f, 2.0f)) * m_cameraCtrl.getCameraToClip();

		glLoadMatrixf(&projection(0, 0));
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&worldToCamera(0, 0));

		Curve curve = m_cameraPath->getDebugCurve();
		glBegin(GL_LINES);
		for (int i = 0; i < curve.size(); ++i) {
			CurvePoint cp = curve[i];
			glVertex3f(cp.V.x, cp.V.y, cp.V.z);
		}
		glEnd();
	}
	gl->checkErrors();
}

void App::renderSceneOptions(GLContext * gl) {

	static const Vec3f BOX_COLORS[4] = {

		Vec3f(0.952f, 0.968f, 0.149f),
		Vec3f(0.772f, 0.117f, 0.007f),
		Vec3f(0.207f, 0.101f, 0.858f),
		Vec3f(0.101f, 0.858f, 0.352f)
	};
	

	Vec2i wndSize = m_window.getSize();
	
	const float sceneBoxSide = float(wndSize.x) / m_allScenes.size();
	
	glDisable(GL_DEPTH_TEST);
	glUseProgram(0);
	glBegin(GL_QUADS);
	float yPos = 1.0f - 2.0f * (float(wndSize.y) - 80.0f) / wndSize.y;
	for (size_t i = 0; i < m_allScenes.size(); ++i) {
		float xPos = 2.0f * float(i) * sceneBoxSide / wndSize.x - 1.0f;
		float xNextPos = 2.0f * float(i+1) * sceneBoxSide / wndSize.x - 1.0f;
		Vec3f color = BOX_COLORS[i % 4];
		glColor3f(color.x, color.y, color.z);
		glVertex2f(xPos, -1.0f);
		glVertex2f(xNextPos, -1.0f);
		glVertex2f(xNextPos, yPos);
		glVertex2f(xPos, yPos);
		
	}
	glEnd();
	
	for (size_t i = 0; i < m_allScenes.size(); ++i) {
		float xPos = 2.0f * float(i) * sceneBoxSide / wndSize.x - 1.0f;
		gl->drawLabel(m_allScenes[i].m_name.c_str(), Vec2f(xPos + 0.1f, yPos-0.05f), 0xFFFFFFFF);
	}

	gl->checkErrors();
}

//------------------------------------------------------------------------

void FW::init(void)
{
    new App;
}

//------------------------------------------------------------------------


void App::readState(StateDump& d)
{

	d.pushOwner("App");
	
	d.popOwner();

	
}

//------------------------------------------------------------------------

void App::writeState(StateDump& d) const
{
	d.pushOwner("App");
	
	d.popOwner();
}

void App::setupScenes() {

	GLContext * gl = m_window.getGL();
	Vec2i sz = m_window.getSize();
	/*
	RayMarchScene * rmScene = new RayMarchScene("shaders/raymarch/shape.glsl", gl);
	m_allScenes.push_back(SceneDescriptor(rmScene, "Shape"));*/

	Timer timer;
	timer.start();

	Mesh<VertexPNTC>*  boxMesh = (Mesh<VertexPNTC>*)importMesh("assets/demo_box/simple_box.obj");
	boxMesh->xform(Mat4f::scale(Vec3f(2.0f)));
	printf("Load box mesh: %f\n", timer.getElapsed());
	timer.end();
	

	timer.start();

	m_alphabet = new AlphabetMesh("assets/alphabet/");
	//Mesh<VertexPNTC>*  helloMesh = m_alphabet->stringify("GREETINGS");
	printf("Load alphabet: %f\n", timer.getElapsed());
	timer.end();
	
	
	timer.start();
	Vec3f lo, hi;
	boxMesh->getBBox(lo, hi);

	std::vector<RTTriangle> triangles;
	std::vector<MeshBase::Material*> materials;
	decomposeMesh(boxMesh, triangles, materials);
	SBVH sbvh(&triangles, 16);

	GPUBvh bvh(&sbvh);
	GPUBvh_Buffers bvhBuffers = bvh.genBuffer(materials);
	printf("Generate bvh: %f\n", timer.getElapsed());
	timer.end();
	


	m_forwardscene = new ForwardShadingScene("forward_shading", gl, sz.x, sz.y);

	m_forwardscene->setMesh(boxMesh);
	

	


	m_allScenes.push_back(SceneDescriptor(m_forwardscene, "Forward"));
	
	// greetings
	timer.start();
	static const std::vector<std::string> Greetings = {
	"GREETINGS!",
	"branch",
	"noby",
	"parallel\nplaces",
	"urs",
	"SPECIAL\nGREETINGS!",
	"JAAKKO\nPAULI\nVILLE",
	"GRAFFATHON <3"
	};
	m_greetings.resize(Greetings.size());
	for (int i = 0; i < Greetings.size(); ++i) {
		
		m_greetings[i] = new LightParticleSystem(m_window.getGL(), bvhBuffers, "shaders/shading_comparison/move_lights.glsl", 800000, m_alphabet->stringify(Greetings[i]));
	}
	
	m_lightSystem = m_greetings[0];
	m_forwardscene->setParticleSystem(m_greetings[0]);
	printf("Generate particles: %f\n", timer.getElapsed());

	timer.end();


	timer.start();
	m_dynamicMesh = new DynamicMarchingTetrahedra(gl, Vec4f(-10.0f, -10.0f, -10.0f, 0.3125f), "shaders/tetrahedra_march/scene.glsl");
	m_forwardscene->setDMTMesh(m_dynamicMesh);
	printf("Dynamic mesh shaders: %f\n", timer.getElapsed());
	timer.end();
	

	timer.start();
	LightParticleSystem * dmParticleSystem = new LightParticleSystem(gl, bvhBuffers, "shaders/shading_comparison/move_lights_vec_field.glsl", m_dynamicMesh->getTriangleVBO());
	m_forwardscene->setDMParticleSystem(dmParticleSystem);
	m_dmLightSystem = dmParticleSystem;
	m_dmLightSystem->setPointSize(9.8f);
	m_scene = m_forwardscene;

	m_cameraPath = new CameraPath("assets/cam_path.txt");
	printf("Load cam paths: %f\n", timer.getElapsed());
	timer.end();
	
}

static void die(const char * message) {
	::printf("%s\n", message);
	exit(1);
}

void App::setupRocket() {
	HWND hwnd = m_window.getHandle();
	if (!BASS_Init(-1, 44100, 0, hwnd, 0)) {
		die("coult not init bass");
	}
	m_stream = BASS_StreamCreateFile(false, "assets/epic_dubstep.ogg", 0, 0, 0);

	if (!m_stream) {
		die("coult not load mp3");
	}

	
	m_rocket = sync_create_device("rocket/sync");

	if (!m_rocket) {
		die("out of memory");
	}

#ifndef SYNC_PLAYER
	sync_set_callbacks(m_rocket, &bass_cb, (void *)m_stream);
	if (sync_connect(m_rocket, "localhost", SYNC_DEFAULT_PORT)) {
		die("failed to connect to host");
	}
#endif

	FWSync::BALL_DISP_1 = sync_get_track(m_rocket, "ball_disp.1");
	FWSync::BALL_DISP_2 = sync_get_track(m_rocket, "ball_disp.2");
	FWSync::BALL_DISP_3 = sync_get_track(m_rocket, "ball_disp.3");
	FWSync::BALL_DISP_4 = sync_get_track(m_rocket, "ball_disp.4");

	FWSync::BALL_DISP_5 = sync_get_track(m_rocket, "ball_disp.5");
	FWSync::BALL_DISP_6 = sync_get_track(m_rocket, "ball_disp.6");
	FWSync::BALL_DISP_7 = sync_get_track(m_rocket, "ball_disp.7");
	FWSync::BALL_DISP_8 = sync_get_track(m_rocket, "ball_disp.8");

	FWSync::BALL_DISP_9 = sync_get_track(m_rocket, "ball_disp.9");
	FWSync::BALL_DISP_10 = sync_get_track(m_rocket, "ball_disp.10");
	FWSync::BALL_DISP_11 = sync_get_track(m_rocket, "ball_disp.11");
	FWSync::BALL_DISP_12 = sync_get_track(m_rocket, "ball_disp.12");

	FWSync::BALL_DISP_13 = sync_get_track(m_rocket, "ball_disp.13");
	FWSync::BALL_DISP_14 = sync_get_track(m_rocket, "ball_disp.14");
	FWSync::BALL_DISP_15 = sync_get_track(m_rocket, "ball_disp.15");
	FWSync::BALL_DISP_16 = sync_get_track(m_rocket, "ball_disp.16");

	FWSync::CLOTH_WIND_FORCE = sync_get_track(m_rocket, "cloth_wind");
	FWSync::CLOTH_Y_POS = sync_get_track(m_rocket, "cloth_y_pos");

	FWSync::BALL_LIGHT_INTENSITY_R = sync_get_track(m_rocket, "ball_light.r");
	FWSync::BALL_LIGHT_INTENSITY_G = sync_get_track(m_rocket, "ball_light.g");
	FWSync::BALL_LIGHT_INTENSITY_B = sync_get_track(m_rocket, "ball_light.b");
	
	FWSync::TEXT_EXPLODE = sync_get_track(m_rocket, "txt_explode");
	FWSync::TEXT_OFFSET = sync_get_track(m_rocket, "txt_offset");
	FWSync::TEXT_SUCK = sync_get_track(m_rocket, "txt_absorb");
	FWSync::TEXT_RESTART = sync_get_track(m_rocket, "txt_restart");
	FWSync::TEXT_ON = sync_get_track(m_rocket, "txt_on");

	FWSync::BOUNCE_TIME = sync_get_track(m_rocket, "b_time");
	FWSync::BOUNCE_PERIOD = sync_get_track(m_rocket, "b_period");
	FWSync::BOUNCE_SCALE = sync_get_track(m_rocket, "b_scale");

	FWSync::DM_MESH_PARTICLE_SWITCH = sync_get_track(m_rocket, "part_switch");
	FWSync::CAM_PATH_TIME = sync_get_track(m_rocket, "cam_time");
	FWSync::BLUR_OUT = sync_get_track(m_rocket, "blur_out");
}

LightParticleSystem * App::getGreeting(int greetingIndex) {

	int idx = greetingIndex % m_greetings.size();
	
	return m_greetings[idx];

}

void App::syncScenes() {
	double row = bass_get_row(m_stream);

#ifndef SYNC_PLAYER
	if (sync_update(m_rocket, (int)floor(row)))
		sync_connect(m_rocket, "localhost", SYNC_DEFAULT_PORT);
#endif



	//Vec3f rgb = Vec3f(sync_get_val(FWSync::RED, row), sync_get_val(FWSync::GREEN, row), sync_get_val(FWSync::BLUE, row));
	Vec4f disp1, disp2, disp3, disp4;
	disp1.x = sync_get_val(FWSync::BALL_DISP_1, row);
	disp1.y = sync_get_val(FWSync::BALL_DISP_2, row);
	disp1.z = sync_get_val(FWSync::BALL_DISP_3, row);
	disp1.w = sync_get_val(FWSync::BALL_DISP_4, row);

	disp2.x = sync_get_val(FWSync::BALL_DISP_5, row);
	disp2.y = sync_get_val(FWSync::BALL_DISP_6, row);
	disp2.z = sync_get_val(FWSync::BALL_DISP_7, row);
	disp2.w = sync_get_val(FWSync::BALL_DISP_8, row);

	disp3.x = sync_get_val(FWSync::BALL_DISP_9, row);
	disp3.y = sync_get_val(FWSync::BALL_DISP_10, row);
	disp3.z = sync_get_val(FWSync::BALL_DISP_11, row);
	disp3.w = sync_get_val(FWSync::BALL_DISP_12, row);

	disp4.x = sync_get_val(FWSync::BALL_DISP_13, row);
	disp4.y = sync_get_val(FWSync::BALL_DISP_14, row);
	disp4.z = sync_get_val(FWSync::BALL_DISP_15, row);
	disp4.w = sync_get_val(FWSync::BALL_DISP_16, row);

	m_dynamicMesh->setDispValues(disp1, disp2, disp3, disp4);

	float clothYPos = sync_get_val(FWSync::CLOTH_Y_POS, row);
	m_forwardscene->setDisplacedMeshNewPos(Vec3f(0, clothYPos, 0));

	

	float txtAbsorb = sync_get_val(FWSync::TEXT_SUCK, row);
	float explode = sync_get_val(FWSync::TEXT_EXPLODE, row);
	float offset = sync_get_val(FWSync::TEXT_OFFSET, row);
	int txtRestart = sync_get_val(FWSync::TEXT_RESTART, row);
	int txtIsOn = sync_get_val(FWSync::TEXT_ON, row);

	static int prevGreetingIndex = 0;

	if (prevGreetingIndex != txtRestart) {
		prevGreetingIndex = txtRestart;
		//m_lightSystem->restartSystem(m_window.getGL(), getGreeting(txtRestart));
		m_lightSystem = getGreeting(txtRestart);
		m_forwardscene->setParticleSystem(m_lightSystem);
		
	}

	m_lightSystem->updateSystemState(offset, explode, txtAbsorb, txtIsOn != 0);

	float bTime = sync_get_val(FWSync::BOUNCE_TIME, row);
	float bPeriod = sync_get_val(FWSync::BOUNCE_PERIOD, row);

	float bScale = sync_get_val(FWSync::BOUNCE_SCALE, row);
	m_forwardscene->updateBounce(bTime, bPeriod, bScale);

	float partSwitch = sync_get_val(FWSync::DM_MESH_PARTICLE_SWITCH, row);

	bool renderDMMesh = true;
	if (partSwitch > 0.0) {
		renderDMMesh = false;
	}
	if (renderDMMesh > 100.0) {
		m_forwardscene->setDMParticleSystem(nullptr);
	}
	m_forwardscene->setDMTRenderState(renderDMMesh);
	m_dmLightSystem->setSceneType(partSwitch);

	float camPathTime = sync_get_val(FWSync::CAM_PATH_TIME, row);

	if (m_followPath) {
		Mat4f toWorld = m_cameraPath->getTransformation(camPathTime);
		m_cameraCtrl.setCameraToWorld(toWorld);
		//m_cameraCtrl.setPosition(m_cameraPath->getPosition(camPathTime));
	}

	float blurOut = sync_get_val(FWSync::BLUR_OUT, row);
	m_forwardscene->setBlurOut(blurOut);

	if (blurOut >= 0.99) {
		exit(1);
	}
}