#include "Game/Car.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/VertexUtils.hpp"

//------------------------------------------------------------------------------------------------------------------------------
Car::Car()
{

}

//------------------------------------------------------------------------------------------------------------------------------
Car::~Car()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void Car::StartUp(const Vec3& startPosition, int controllerID, float timeToBeat)
{
	m_timeToBeat = timeToBeat;

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

	m_HUDFont = g_renderContext->CreateOrGetBitmapFontFromFile("AtariClassic", VARIABLE_WIDTH);
	m_HUDshader = g_renderContext->CreateOrGetShaderFromFile(m_shaderPath);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::Update(float deltaTime, bool isInputEnabled /*= true*/)
{
	if (isInputEnabled)
	{
		m_controller->Update(deltaTime);
		m_controller->m_controlReleased = false;
	}
	else
	{
		m_controller->ReleaseAllControls();
		m_controller->m_controlReleased = true;
	}
	
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
void Car::RenderUIHUD() const
{
	g_renderContext->BeginCamera(*m_carHUD);

	//Draw all the background boxes
	g_renderContext->BindTextureViewWithSampler(0U, nullptr);
	g_renderContext->BindShader(m_HUDshader);

	RenderBackgroundBoxes();

	g_renderContext->BindTextureViewWithSampler(0U, m_HUDFont->GetTexture(), SAMPLE_MODE_POINT);

	RenderLapCounter();
	RenderTimeTaken();
	RenderTimeToBeat();
	RenderGearIndicator();
	RenderRevMeter();

	g_renderContext->EndCamera();
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::RenderBackgroundBoxes() const
{
	//We want to render all the background boxes for the UI HUD
	std::vector<Vertex_PCU> boxVerts;

	Vec2 camMinBounds = m_carHUD->GetOrthoBottomLeft();
	Vec2 camMaxBounds = m_carHUD->GetOrthoTopRight();

	Rgba backgroundColor = Rgba::ORGANIC_DIM_BLUE;
	backgroundColor.a = 0.5f;

	AABB2 box;
	
	//Lower left corner
	box.m_minBounds = Vec2::ZERO;
	box.m_maxBounds = Vec2(80.f, 15.f);
	AddVertsForAABB2D(boxVerts, box, backgroundColor);

	//Lower right corner
	box.m_minBounds = camMaxBounds;
	box.m_minBounds.x -= 100.f;
	box.m_minBounds.y = 0.0f;
	box.m_maxBounds = camMaxBounds;
	box.m_maxBounds.y = 15.f;
	AddVertsForAABB2D(boxVerts, box, backgroundColor);

	//Upper left corner
	box.m_minBounds = camMaxBounds;
	box.m_minBounds.x = 0.f;
	box.m_minBounds.y -= 15.0f;
	box.m_maxBounds = camMaxBounds;
	box.m_maxBounds.x = 80.f;
	
	AddVertsForAABB2D(boxVerts, box, backgroundColor);

	//Upper right corner
	box.m_minBounds = camMaxBounds;
	box.m_minBounds.x -= 100.f;
	box.m_minBounds.y -= 15.0f;
	box.m_maxBounds = camMaxBounds;
	AddVertsForAABB2D(boxVerts, box, backgroundColor);

	g_renderContext->DrawVertexArray(boxVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::RenderLapCounter() const
{
	//Render the current lap out of num total laps
	int lapNumber = m_waypoints.GetCurrentLapNumber();
	int numLaps = m_waypoints.GetMaxLapCount();

	Vec2 camMinBounds = m_carHUD->GetOrthoBottomLeft();
	Vec2 camMaxBounds = m_carHUD->GetOrthoTopRight();

	Vec2 displayArea = camMaxBounds;
	displayArea.x = 20.f;
	displayArea.y -= m_HUDFontHeight * 2.f;

	std::string printString = Stringf("Laps: %d/%d", lapNumber, numLaps);
	std::vector<Vertex_PCU> textVerts;

	if (!m_waypoints.AreLapsComplete())
	{
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::WHITE);
	}
	else
	{
		std::string printString = "COMPLETE!";
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::ORGANIC_GREEN);
	}

	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::RenderTimeTaken() const
{
	float timeTaken = m_waypoints.GetTotalTime();

	Vec2 camMinBounds = m_carHUD->GetOrthoBottomLeft();
	Vec2 camMaxBounds = m_carHUD->GetOrthoTopRight();

	Vec2 displayArea = camMaxBounds;
	displayArea.x -= 100.f;
	displayArea.y -= m_HUDFontHeight * 2.f;

	std::string printString = Stringf("Time Taken: %.3f", timeTaken);
	std::vector<Vertex_PCU> textVerts;

	if (!m_waypoints.AreLapsComplete())
	{
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::WHITE);
	}
	else
	{
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::ORGANIC_GREEN);
	}

	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::RenderTimeToBeat() const
{
	float timeToBeat = m_timeToBeat;

	Vec2 camMinBounds = m_carHUD->GetOrthoBottomLeft();
	Vec2 camMaxBounds = m_carHUD->GetOrthoTopRight();

	Vec2 displayArea = camMaxBounds;
	displayArea.x -= 100.f;
	displayArea.y = 0.f;
	displayArea.y += m_HUDFontHeight;

	std::string printString = Stringf("Time To Beat: %.3f", timeToBeat);
	std::vector<Vertex_PCU> textVerts;

	if (!m_waypoints.AreLapsComplete())
	{
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::WHITE);
	}
	else if(m_waypoints.AreLapsComplete() && m_waypoints.GetTotalTime() < m_timeToBeat)
	{
		std::string printString = Stringf("Time To Beat: %.3f", m_waypoints.GetTotalTime());
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::ORGANIC_GREEN);
	}
	else
	{
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::ORGANIC_RED);
	}

	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::RenderGearIndicator() const
{
	int currentGear = m_controller->GetVehicle()->mDriveDynData.getCurrentGear();

	Vec2 camMinBounds = m_carHUD->GetOrthoBottomLeft();
	Vec2 camMaxBounds = m_carHUD->GetOrthoTopRight();

	Vec2 displayArea = camMinBounds;
	displayArea.x += 20.f;
	displayArea.y += m_HUDFontHeight;

	std::string printString = Stringf("Gear: %d", currentGear - 1);
	std::vector<Vertex_PCU> textVerts;

	if (currentGear != 1 && currentGear != 0)
	{
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::WHITE);
	}
	else if(currentGear == 1)
	{
		printString = "Gear: N";
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::ORGANIC_GREEN);
	}
	else
	{
		printString = "Gear: R";
		m_HUDFont->AddVertsForText2D(textVerts, displayArea, m_HUDFontHeight, printString, Rgba::ORGANIC_ORANGE);
	}

	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Car::RenderRevMeter() const
{

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
void Car::ResetWaypointSystem()
{
	m_waypoints.Reset();
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

