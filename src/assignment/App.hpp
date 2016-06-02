#pragma once

#include "Scene.h"
#include "Action.h"

#include "gui/Window.hpp"
#include "gui/CommonControls.hpp"
#include "3d/CameraControls.hpp"
#include "3d/Texture.hpp"
#include "gpu/Buffer.hpp"
#include "gui/Image.hpp"
#include "DisplacedMesh.h"
#include "LightParticles.h"
#include "ForwardShading.h"
#include "SnakeTrail.h"
#include "bass.h"
#include "sync.h"
#include "AlphabetMesh.h"
#include "DynamicMarchingTetrahedra.h"
#include "CameraPath.h"
#include <vector>

namespace FW
{
//------------------------------------------------------------------------

class App : public Window::Listener, public CommonControls::StateObject
{

public:
                    App             (void);
    virtual         ~App            (void);

    virtual bool    handleEvent     (const Window::Event& ev);
	virtual void    readState(StateDump& d);
	virtual void    writeState(StateDump& d) const;

private:
    void            waitKey         (void);
    void            renderFrame     (GLContext* gl);
	void			renderSceneOptions(GLContext * gl);

private:
                    App             (const App&); // forbidden
    App&            operator=       (const App&); // forbidden

private:
    Window          m_window;
    CommonControls  m_commonCtrl;
    CameraControls  m_cameraCtrl;

    Action          m_action;
	Knob			m_activeKnob;
	Knob			m_prevKnob;
	
	bool m_displaySceneTabs;
	Scene * m_scene;
	Mesh<VertexPNTC>* m_icosahedron;

	std::vector<SceneDescriptor> m_allScenes;
	LightParticleSystem * m_lightSystem;
	LightParticleSystem * m_dmLightSystem;
	ForwardShadingScene *m_forwardscene;
	SnakeTrail * m_snake;
	void setupScenes();
	void setupRocket();

	void syncScenes();

	LightParticleSystem * getGreeting(int greetingIndex);

	// bass stuff
	HSTREAM m_stream;
	sync_device * m_rocket;

	DisplacedMesh * m_centerMesh;
	AlphabetMesh * m_alphabet;
	DynamicMarchingTetrahedra * m_dynamicMesh;
	std::vector<LightParticleSystem*> m_greetings;
	CameraPath * m_cameraPath;
	bool m_followPath;
	bool m_renderPath;
};

static const float bpm = 150.0f; /* beats per minute */
static const float rpb = 0.9; /* rows per beat */
static const double row_rate = (double(bpm) / 60) * rpb;
#ifndef SYNC_PLAYER
static void bass_pause(void *d, int flag)
{
	if (flag)
		BASS_ChannelPause((HSTREAM)d);
	else
		BASS_ChannelPlay((HSTREAM)d, false);
}

static void bass_set_row(void *d, int row)
{
	QWORD pos = BASS_ChannelSeconds2Bytes((HSTREAM)d, row / row_rate);
	BASS_ChannelSetPosition((HSTREAM)d, pos, BASS_POS_BYTE);
}

static int bass_is_playing(void *d)
{
	return BASS_ChannelIsActive((HSTREAM)d) == BASS_ACTIVE_PLAYING;
}

struct sync_cb bass_cb = {
	bass_pause,
	bass_set_row,
	bass_is_playing
};
#endif

static double bass_get_row(HSTREAM h)
{
	QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_BYTE);
	double time = BASS_ChannelBytes2Seconds(h, pos);
	return time * row_rate;
}

//------------------------------------------------------------------------
}
