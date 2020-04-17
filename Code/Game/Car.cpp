#include "Game/Car.hpp"
#include "Engine/Math/MathUtils.hpp"

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
	m_carHUD = new Camera();
	m_controller = new CarController();
	m_audio = new CarAudio(m_controller);
	m_audio->Startup();

	m_controller->SetVehiclePosition(startPosition);
	m_controller->SetControllerIDToUse(controllerID);

	m_carIndex = controllerID;

	m_carHUD->SetColorTarget(nullptr);

	Vec2 orthoBottomLeft = Vec2(0.f, 0.f);
	Vec2 orthoTopRight = Vec2(m_HUD_WIDTH, m_HUD_HEIGHT);
	m_carHUD->SetOrthoView(orthoBottomLeft, orthoTopRight);

}

//------------------------------------------------------------------------------------------------------------------------------
void Car::Update(float deltaTime)
{
	m_controller->Update(deltaTime);
	m_audio->Update();

	//Update the waypoint system
	if (m_waypoints.AreLapsComplete())
	{
		m_raceTime = m_waypoints.GetTotalTime();
	}
	else
	{
		m_waypoints.Update(m_controller->GetVehiclePosition());
		m_raceTime = m_waypoints.GetTotalTime();
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::FixedUpdate(float fixedTime)
{
	m_controller->FixedUpdate(fixedTime);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::Shutdown()
{
	delete m_camera;
	m_camera = nullptr;

	delete m_carHUD;
	m_carHUD = nullptr;

	delete m_controller;
	m_controller = nullptr;

	delete m_audio;
	m_audio = nullptr;
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
CarController* Car::GetCarControllerEditable()
{
	return m_controller;
}

//------------------------------------------------------------------------------------------------------------------------------
const CarAudio& Car::GetCarAudio() const
{
	return *m_audio;
}

//------------------------------------------------------------------------------------------------------------------------------
CarAudio* Car::GetCarAudioEditable()
{
	return m_audio;
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
Camera& Car::GetCarHUDCamera() const
{
	return *m_carHUD;
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

//------------------------------------------------------------------------------------------------------------------------------
void Car::SetupNewPlaybackIDs()
{
	m_audio->SetNewPlaybackIDs();
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
double Car::GetRaceTime()
{
	return m_raceTime;
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::ResetCarPosition()
{
	//Vec3 vehiclePosition = m_controller->GetVehiclePosition();
	PxTransform transform = m_controller->GetVehicle()->getRigidDynamicActor()->getGlobalPose();

	Matrix44 vehicleMatrix = PhysXSystem::MakeMatrixFromQuaternion(transform.q, transform.p);

	vehicleMatrix.MakeXRotationDegrees(0.f);
	vehicleMatrix.MakeZRotationDegrees(0.f);
	vehicleMatrix.SetTranslation3D(Vec3(transform.p.x, transform.p.y + m_resetHeight, transform.p.z), vehicleMatrix);

	PxQuat resultQuaternion = PhysXSystem::MakeQuaternionFromMatrix(vehicleMatrix);

	transform.q = resultQuaternion;
	Vec3 position = vehicleMatrix.GetTBasis();

	transform.p = PhysXSystem::VecToPxVector(position);

	m_controller->SetVehicleTransform(transform);

// 	Vec3 vehicleForward = m_controller->GetVehicleForwardBasis();
// 
// 
// 	Vec3 forwardOnZXPlane = vehicleForward;
// 	forwardOnZXPlane.y = 0.f;
// 	forwardOnZXPlane.Normalize();
// 	//Set (y) on forward vector to 0
// 	//Re-normalize forward
// 
// 	Vec3 vehicleRight = m_controller->GetVehicleRightBasis();
// 
// 	PxQuat q = PhysXSystem::MakeQuaternionFromVectors(vehicleForward, forwardOnZXPlane);
// 
// 	//Move up by some amount
// 	vehiclePosition.y += m_resetHeightAdd;
// 	m_controller->SetVehicleTransform(vehiclePosition, q);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::SetupCarAudio()
{
	if (m_carIndex == 0)
	{
		for (int index = 0; index < CAR_FILE_PATHS.size(); index++)
		{
			CAR_FILE_PATHS[index] = m_BASE_AUDIO_PATH + CAR_FILE_PATHS[index];
		}
	}

	m_audio->InitializeFromPaths(CAR_FILE_PATHS);
}

