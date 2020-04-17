#pragma once
#include "Game/CarController.hpp"
#include "Game/CarCamera.hpp"
#include "Game/CarAudio.hpp"
#include "Game/WaypointSystem.hpp"
//Engine Systems
#include "Engine/Renderer/BitmapFont.hpp"

static std::vector<std::string> CAR_FILE_PATHS = {
	"/C_5_ExhL_02142.wav",
	"/C_6_ExhL_02412.wav",
	"/C_9_ExhL_03775.wav",
	"/C_10_ExhL_04308.wav",
	"/C_11_ExhL_04984.wav",
	"/C_12_ExhL_05652.wav",
	"/C_13_ExhL_06177.wav",
	"/C_14_ExhL_06669.wav"
};

//------------------------------------------------------------------------------------------------------------------------------
class Car
{
public:
	Car();
	~Car();

	void						StartUp(const Vec3& startPosition, int controllerID);
	void						Update(float deltaTime, bool isInputEnabled = true);
	void						FixedUpdate(float fixedTime);
	void						Shutdown();

	void						SetupCarAudio();

	const CarCamera&			GetCarCamera() const;
	CarCamera*					GetCarCameraEditable();
	const CarController&		GetCarController() const;
	CarController*				GetCarControllerEditable();
	const CarAudio&				GetCarAudio() const;
	CarAudio*					GetCarAudioEditable();
	int							GetCarIndex() const;
	PxRigidDynamic*				GetCarRigidbody() const;
	Camera&						GetCarHUDCamera() const;

	WaypointSystem&				GetWaypointsEditable();
	const WaypointSystem&		GetWaypoints() const;

	void						SetupNewPlaybackIDs();

	//For reset car transform we will just orient it at current forward direction and set position at current pos and y += 10;
	void						ResetCarPosition();

	void						SetCameraColorTarget(ColorTargetView* colorTargetView);
	void						SetCameraPerspectiveProjection(float m_camFOVDegrees, float nearZ, float farZ, float aspect);
	void						UpdateCarCamera(float deltaTime);

	double						GetRaceTime();

private:

private:
	CarController*				m_controller = nullptr;
	CarCamera*					m_camera = nullptr;
	CarAudio*					m_audio = nullptr;
	Camera*						m_carHUD = nullptr;

	WaypointSystem				m_waypoints;

	const std::string			m_BASE_AUDIO_PATH = "Data/Audio/Ferrari944";

	int							m_carIndex = -1;

	float						m_HUD_WIDTH = 300.f;
	float						m_HUD_HEIGHT = 150.f;

	BitmapFont*					m_HUDFont = nullptr;

	double						m_raceTime = 0.0f;
	float						m_resetHeight = 2.0f;
};