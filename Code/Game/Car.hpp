#pragma once
#include "Game/CarController.hpp"
#include "Game/CarCamera.hpp"
#include "Game/CarAudio.hpp"
#include "Game/WaypointSystem.hpp"

static std::vector<std::string> CAR_FILE_PATHS = {
	"/C_1_ExhL_00948.wav",
	"/C_2_ExhL_01338.wav",
	"/C_3_ExhL_01610.wav",
	"/C_4_ExhL_01866.wav",
	"/C_5_ExhL_02142.wav",
	"/C_6_ExhL_02412.wav",
	"/C_7_ExhL_02673.wav",
	"/C_8_ExhL_03234.wav",
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
	void						Update(float deltaTime);
	void						FixedUpdate(float fixedTime);

	const CarCamera&			GetCarCamera() const;
	CarCamera*					GetCarCameraEditable();
	const CarController&		GetCarController() const;
	int							GetCarIndex() const;
	PxRigidDynamic*				GetCarRigidbody() const;

	WaypointSystem&				GetWaypointsEditable();
	const WaypointSystem&		GetWaypoints() const;

	void						SetCameraColorTarget(ColorTargetView* colorTargetView);
	void						SetCameraPerspectiveProjection(float m_camFOVDegrees, float nearZ, float farZ, float aspect);
	void						UpdateCarCamera(float deltaTime);

private:
	
	void						SetupCarAudio();

private:
	CarController*				m_controller = nullptr;
	CarCamera*					m_camera = nullptr;
	CarAudio*					m_audio = nullptr;

	WaypointSystem				m_waypoints;

	const std::string			m_BASE_AUDIO_PATH = "Data/Audio/Ferrari944";

	int							m_carIndex = -1;
};