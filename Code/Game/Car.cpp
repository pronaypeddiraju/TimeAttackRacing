#include "Game/Car.hpp"

//------------------------------------------------------------------------------------------------------------------------------
Car::Car()
{

}

//------------------------------------------------------------------------------------------------------------------------------
Car::~Car()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void Car::StartUp(const Vec3& startPosition, int controllerID)
{
	m_camera = new CarCamera();
	m_controller = new CarController();
	m_audio = new CarAudio(m_controller);

	m_controller->SetVehiclePosition(startPosition);
	m_controller->SetControllerIDToUse(controllerID);

	m_carIndex = controllerID;

	SetupCarAudio();
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::Update(float deltaTime)
{
	m_controller->Update(deltaTime);
	m_audio->Update();

	//Update the waypoint system
	m_waypoints.Update(m_controller->GetVehiclePosition());

	
// 	float radiansPerSecond = m_controller->GetVehicle()->mDriveDynData.getEngineRotationSpeed() * 60 * 0.5f / PxPi;
// 	float maxRadsPerSecond = m_controller->GetVehicle()->mDriveSimData.getEngineData().mMaxOmega * 60 * 0.5f / PxPi;
// 	//float RPM = PhysXSystem::GetRadiansPerSecondToRotationsPerMinute(radiansPerSecond) * (1000.f / maxRadsPerSecond);
// 	float RPM = radiansPerSecond / maxRadsPerSecond;
// 	RPM = RPM * m_controller->GetVehicle()->mDriveDynData.getCurrentGear();
// 
// 	DebuggerPrintf("\n %f", RPM * 100.f);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::FixedUpdate(float fixedTime)
{
	m_controller->FixedUpdate(fixedTime);
}

//------------------------------------------------------------------------------------------------------------------------------
const CarCamera& Car::GetCarCamera() const
{
	return *m_camera;
}

//------------------------------------------------------------------------------------------------------------------------------
CarCamera* Car::GetCarCameraEditable()
{
	return m_camera;
}

//------------------------------------------------------------------------------------------------------------------------------
const CarController& Car::GetCarController() const
{
	return *m_controller;
}

//------------------------------------------------------------------------------------------------------------------------------
int Car::GetCarIndex() const
{
	return m_carIndex;
}

//------------------------------------------------------------------------------------------------------------------------------
physx::PxRigidDynamic* Car::GetCarRigidbody() const
{
	return m_controller->GetVehicle()->getRigidDynamicActor();
}

//------------------------------------------------------------------------------------------------------------------------------
WaypointSystem& Car::GetWaypointsEditable()
{
	return m_waypoints;
}

//------------------------------------------------------------------------------------------------------------------------------
const WaypointSystem& Car::GetWaypoints() const
{
	return m_waypoints;
}

void Car::SetCameraColorTarget(ColorTargetView* colorTargetView)
{
	m_camera->SetColorTarget(colorTargetView);
}

void Car::SetCameraPerspectiveProjection(float m_camFOVDegrees, float nearZ, float farZ, float aspect)
{
	m_camera->SetPerspectiveProjection(m_camFOVDegrees, nearZ, farZ, aspect);
}

void Car::UpdateCarCamera(float deltaTime)
{
	Vec3 carPos = m_controller->GetVehiclePosition();
	m_camera->SetFocalPoint(carPos);

	Vec3 carForward = m_controller->GetVehicleForwardBasis();

	m_camera->Update(carForward, deltaTime);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::SetupCarAudio()
{
	for (int index = 0; index < CAR_FILE_PATHS.size(); index++)
	{
		CAR_FILE_PATHS[index] = m_BASE_AUDIO_PATH + CAR_FILE_PATHS[index];
	}

	m_audio->InitializeFromPaths(CAR_FILE_PATHS);
}

