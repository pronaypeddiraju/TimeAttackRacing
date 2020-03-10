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

	m_controller->SetVehiclePosition(startPosition);
	m_controller->SetControllerIDToUse(controllerID);

	m_carIndex = controllerID;
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::Update(float deltaTime)
{
	m_controller->Update(deltaTime);
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

