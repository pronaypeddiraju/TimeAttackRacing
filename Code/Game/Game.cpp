//------------------------------------------------------------------------------------------------------------------------------
#include "Game/Game.hpp"
//Engine Systems
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Commons/Profiler/Profiler.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystems.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Vertex_Lit.hpp"
#include "Engine/PhysXSystem/PhysXSystem.hpp"
#include "Engine/PhysXSystem/PhysXVehicleFilterShader.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/ColorTargetView.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/ObjectLoader.hpp"
#include "Engine/Core/FileUtils.hpp"
//Game Systems
#include "Game/UIWidget.hpp"
//Third party
#include "ThirdParty/TinyXML2/tinyxml2.h"
#include <fstream>

//------------------------------------------------------------------------------------------------------------------------------
float g_shakeAmount = 0.0f;

extern RenderContext* g_renderContext;
extern AudioSystem* g_audio;
bool g_debugMode = false;

//------------------------------------------------------------------------------------------------------------------------------
Game::Game()
{
	m_isGameAlive = true;

	m_menuFont = g_renderContext->CreateOrGetBitmapFontFromFile("AtariClassic", VARIABLE_WIDTH);

	g_devConsole->SetBitmapFont(*m_menuFont);
	g_debugRenderer->SetDebugFont(m_menuFont);
}

//------------------------------------------------------------------------------------------------------------------------------
Game::~Game()
{
	m_isGameAlive = false;
	Shutdown();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::StartUp()
{
	SetupMouseData();

	GetandSetShaders();
	LoadGameTextures();
	LoadGameMaterials();
	CreateUIWidgets();

	g_devConsole->PrintString(Rgba::ORGANIC_BLUE, "This is the Dev Console");

	CreateInitialLight();
	SetupCameras();
	SetupPhysX();

	Vec3 camEuler = Vec3(-12.5f, -196.f, 0.f);
	m_mainCamera->SetEuler(camEuler);

	ReadBestTimeFromTextFile();

	CreateInitialMeshes();
	//LoadGameTexturesThreaded();
	//PerformAsyncLoading();

	PerformSingleThreadLoading();

	//Load Audio
	LoadAudio();

	m_textureTest = g_renderContext->CreateOrGetTextureViewFromFile("Data/Images/seamLessRoad.png");

	//Set this to true for now since we are only single threaded
	m_threadedLoadComplete = true;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadAudio()
{
	m_BGMTrack = g_audio->CreateOrGetSound(m_BGMPath);
	m_BGMTrackPlaybackID = g_audio->PlayAudio(m_BGMTrack, true);
	g_audio->SetSoundPlaybackVolume(m_BGMTrackPlaybackID, 0.2f);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::InitiateGameSequence()
{
	//called once when m_initiateFromMenu is set to true

	//Call InputSystem frame to detect xBox controllers
	g_inputSystem->BeginFrame();
	m_numConnectedPlayers = g_inputSystem->GetNumConnectedControllers();
	g_inputSystem->EndFrame();

	//Setup the cars	
	SetupCars();

	CreateWayPoints();

	SetEnableXInput(true);

	CreateBaseBoxForCollisionDetection();

	LoadTrackMeshesOnSceneCreation();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetupCars()
{
	IntVec2 client = g_windowContext->GetTrueClientBounds();
	float aspect = (float)client.x / (float)client.y;

	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		m_cars[carIndex] = new Car();
		m_cars[carIndex]->StartUp(m_startPositions[carIndex], carIndex, m_bestTimeFromFile);
		m_cars[carIndex]->SetupCarAudio();

		m_cars[carIndex]->SetCameraColorTarget(nullptr);
		m_cars[carIndex]->SetCameraPerspectiveProjection(m_camFOVDegrees, 0.1f, 1000.f, aspect);

		m_splitScreenSystem.AddCarCameraForPlayer(m_cars[carIndex]->GetCarCameraEditable(), m_cars[carIndex]->GetCarIndex());
	}

}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetupMouseData()
{
	g_windowContext->SetMouseMode(MOUSE_MODE_ABSOLUTE);
	//g_windowContext->HideMouse();
}

void Game::SetupCameras()
{
	IntVec2 client = g_windowContext->GetTrueClientBounds();
	float aspect = (float)client.x / (float)client.y;

	//Create the Camera and setOrthoView
	m_mainCamera = new Camera();
	m_mainCamera->SetColorTarget(nullptr);

	//Create a devConsole Cam
	m_devConsoleCamera = new Camera();
	m_devConsoleCamera->SetColorTarget(nullptr);

	//Create the UI Camera
	m_UICamera = new Camera();
	m_UICamera->SetColorTarget(nullptr);
	
	//Create the world bounds AABB2
	Vec2 minWorldBounds = Vec2::ZERO;
	Vec2 maxWorldBounds = Vec2((float)client.x, (float)client.y);
	m_UIBounds = AABB2(minWorldBounds, maxWorldBounds);

	//Set Projection Perspective for new Cam
	m_camPosition = Vec3(30.f, 30.f, 60.f);
	m_mainCamera->SetColorTarget(nullptr);
	m_mainCamera->SetPerspectiveProjection( m_camFOVDegrees, 0.1f, 1000.0f, aspect);

	m_UICamera->SetOrthoView(minWorldBounds, maxWorldBounds);
	m_devConsoleCamera->SetOrthoView(minWorldBounds, maxWorldBounds);

	m_clearScreenColor = new Rgba(0.755f, 0.964f, 1.f, 1.f);
}

void Game::SetStartupDebugRenderObjects()
{
	ColorTargetView* ctv = g_renderContext->GetFrameColorTarget();
	//Setup debug render client data
	g_debugRenderer->SetClientDimensions( ctv->m_height, ctv->m_width );

	//Setup Debug Options
	DebugRenderOptionsT options;
	options.mode = DEBUG_RENDER_ALWAYS;
	options.beginColor = Rgba::BLUE;
	options.endColor = Rgba::RED;

	//------------------------------------------------------------------------------------------------------------------------------
	// 2D Objects
	//------------------------------------------------------------------------------------------------------------------------------

	//Make 2D Point on screen
	g_debugRenderer->DebugRenderPoint2D(options, Vec2(10.f, 10.f), 5.0f);
	//Make 2D Point at screen center
	options.beginColor = Rgba::BLUE;
	options.endColor = Rgba::BLACK;
	g_debugRenderer->DebugRenderPoint2D(options, Vec2(0.f, 0.f), 10.f);

	options.beginColor = Rgba::YELLOW;
	options.endColor = Rgba::RED;
	//Draw a line in 2D screen space
	g_debugRenderer->DebugRenderLine2D(options, Vec2(ctv->m_width * -0.5f, ctv->m_height * -0.5f), Vec2(-150.f, -150.f), 20.f);

	//Draw a quad in 2D screen space
	options.beginColor = Rgba::GREEN;
	options.endColor = Rgba::RED;
	g_debugRenderer->DebugRenderQuad2D(options, AABB2(Vec2(-150.f, -150.f), Vec2(-100.f, -100.f)), 20.f);

	//Textured Quad
	options.beginColor = Rgba::WHITE;
	options.endColor = Rgba::RED;
	g_debugRenderer->DebugRenderQuad2D(options, AABB2(Vec2(-200.f, -200.f), Vec2(-150.f, -150.f)), 20.f, m_textureTest);

	//Disc2D
	options.beginColor = Rgba::DARK_GREY;
	options.endColor = Rgba::ORANGE;
	g_debugRenderer->DebugRenderDisc2D(options, Disc2D(Vec2(100.f, 100.f), 25.f), 10.f);

	//Ring2D
	options.beginColor = Rgba::ORANGE;
	options.endColor = Rgba::DARK_GREY;
	g_debugRenderer->DebugRenderRing2D(options, Disc2D(Vec2(100.f, 100.f), 25.f), 10.f, 5.f);

	//Ring2D
	options.beginColor = Rgba::WHITE;
	options.endColor = Rgba::WHITE;
	g_debugRenderer->DebugRenderRing2D(options, Disc2D(Vec2(150.f, 100.f), 2000.f), 10.f, 1.f);

	//Wired Quad
	options.beginColor = Rgba::WHITE;
	options.endColor = Rgba::WHITE;
	g_debugRenderer->DebugRenderWireQuad2D(options, AABB2(Vec2(100.f, -100.f), Vec2(150.f, -50.f)), 20.f);

	//Text
	options.beginColor = Rgba::WHITE;
	options.endColor = Rgba::RED;
	const char* text2D = "Read me bruh";
	g_debugRenderer->DebugRenderText2D(options, Vec2(-100.f, 200.f), Vec2(100.f, 200.f), text2D, DEFAULT_TEXT_HEIGHT, 20.f);

	//Arrow 2D
	options.beginColor = Rgba::GREEN;
	options.endColor = Rgba::GREEN;
	g_debugRenderer->DebugRenderArrow2D(options, Vec2(0.f, 0.f), Vec2(200.f, 200.f), 20.f, 5.f);

	//Arrow 2D
	options.beginColor = Rgba::BLUE;
	options.endColor = Rgba::BLUE;
	g_debugRenderer->DebugRenderArrow2D(options, Vec2(0.f, 0.f), Vec2(200.f, -200.f), 20.f, 5.f);

	//------------------------------------------------------------------------------------------------------------------------------
	// 3D Objects
	//------------------------------------------------------------------------------------------------------------------------------
	
	DebugRenderOptionsT options3D;
	options3D.space = DEBUG_RENDER_WORLD;
	options3D.beginColor = Rgba::GREEN;
	options3D.endColor = Rgba::RED;

	options3D.mode = DEBUG_RENDER_XRAY;
	//make a 3D point
	g_debugRenderer->DebugRenderPoint(options3D, Vec3(0.0f, 0.0f, 0.0f), 10000.0f );

	options3D.mode = DEBUG_RENDER_USE_DEPTH;
	//Make a 3D textured point
	options3D.beginColor = Rgba::BLUE;
	options3D.endColor = Rgba::RED;
	g_debugRenderer->DebugRenderPoint(options3D, Vec3(-10.0f, 0.0f, 0.0f), 20.f, 1.f, m_textureTest);

	options3D.mode = DEBUG_RENDER_XRAY;
	//Make a line in 3D
	options3D.beginColor = Rgba::BLUE;
	options3D.endColor = Rgba::BLACK;
	g_debugRenderer->DebugRenderLine(options3D, Vec3(0.f, 0.f, 5.f), Vec3(10.f, 0.f, 10.f), 2000.f);

	options3D.mode = DEBUG_RENDER_USE_DEPTH;
	//Make a line in 3D
	options3D.beginColor = Rgba::BLUE;
	options3D.endColor = Rgba::BLACK;
	g_debugRenderer->DebugRenderLine(options3D, Vec3(0.f, 0.f, 5.f), Vec3(10.f, 0.f, 10.f), 2000.f);

	//Make a sphere
	options3D.beginColor = Rgba::RED;
	options3D.endColor = Rgba::BLACK;
	g_debugRenderer->DebugRenderSphere(options3D, Vec3(0.f, 3.f, 0.f), 1.f, 10.f, nullptr);
	
	//Make a sphere
	options3D.beginColor = Rgba::GREEN;
	options3D.endColor = Rgba::WHITE;
	g_debugRenderer->DebugRenderSphere(options3D, Vec3(0.f, -3.f, 0.f), 1.f, 200.f, m_sphereTexture);
	
	//Make a wire sphere
	options3D.beginColor = Rgba::WHITE;
	options3D.endColor = Rgba::WHITE;
	g_debugRenderer->DebugRenderWireSphere(options3D, Vec3(0.f, -2.f, 0.f), 1.f, 200.f);
	
	//Make a cube
	options3D.beginColor = Rgba::DARK_GREY;
	options3D.endColor = Rgba::WHITE;
	AABB3 cube = AABB3::UNIT_CUBE;
	g_debugRenderer->DebugRenderBox(options3D, cube, Vec3(-5.f, -1.5f, 0.f), 20.f);

	//Make a wire cube
	options3D.beginColor = Rgba::DARK_GREY;
	options3D.endColor = Rgba::WHITE;
	g_debugRenderer->DebugRenderWireBox(options3D, cube, Vec3(-5.f, 1.5f, 0.f), 20.f);

	//Make a quad 3D no billboard
	options3D.beginColor = Rgba::WHITE;
	options3D.endColor = Rgba::RED;
	AABB2 quad = AABB2(Vec3(-1.f, -1.f, 0.f), Vec3(1.f, 1.f, 0.f));
	Vec3 position = Vec3(3.f, 2.f, 1.f);
	g_debugRenderer->DebugRenderQuad(options3D, quad, position, 2000.f, m_textureTest, false);

	//Make a quad 3D 
	options3D.beginColor = Rgba::WHITE;
	options3D.endColor = Rgba::RED;
	quad = AABB2(Vec3(-1.f, -1.f, 0.f), Vec3(1.f, 1.f, 0.f));
	position = Vec3(5.f, 2.f, 1.f);
	g_debugRenderer->DebugRenderQuad(options3D, quad, position, 2000.f, m_textureTest);

	//Make text
	options3D.beginColor = Rgba::WHITE;
	options3D.endColor = Rgba::RED;
	const char* text = "This is some text";
	g_debugRenderer->DebugRenderText3D(options3D, Vec3(1.f, 1.f, 1.f), Vec2(1.f, 1.f), text, 0.1f, 20000.f);

	//Make text non billboarded
	options3D.beginColor = Rgba::BLUE;
	options3D.endColor = Rgba::RED;
	const char* textNB = "Billboard this";
	g_debugRenderer->DebugRenderText3D(options3D, Vec3(1.f, 0.5f, 0.f), Vec2(-1.f, 1.f), textNB, 0.2f, 20000.f, false);

	//------------------------------------------------------------------------------------------------------------------------------
	//	LOG Objects
	//------------------------------------------------------------------------------------------------------------------------------
	//Setup Debug Options
	options.mode = DEBUG_RENDER_ALWAYS;
	options.beginColor = Rgba::WHITE;
	options.endColor = Rgba::YELLOW;
	const char* debugText1 = "Debug Log Test";
	g_debugRenderer->DebugAddToLog(options, debugText1, Rgba::YELLOW, 10.f);

	//Setup Debug Options
	options.beginColor = Rgba::WHITE;
	options.endColor = Rgba::GREEN;
	const char* debugText2 = "This is another Debug String";
	g_debugRenderer->DebugAddToLog(options, debugText2, Rgba::GREEN, 20.f);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetupPhysX()
{
	PxPhysics* physX = g_PxPhysXSystem->GetPhysXSDK();
	PxScene* pxScene = g_PxPhysXSystem->GetPhysXScene();

	PxMaterial* pxMat;
	pxMat = g_PxPhysXSystem->GetDefaultPxMaterial();

	//Add things to your scene
	PxRigidStatic* groundPlane = PxCreatePlane(*physX, PxPlane(0, 1, 0, 0), *pxMat);
	pxScene->addActor(*groundPlane);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreateUIWidgets()
{
	IntVec2 clientSize = g_windowContext->GetTrueClientBounds();

	// Menu Widgets
	m_menuParent = new UIWidget(this, nullptr);
	m_menuParent->SetColor(Rgba(0.f, 0.f, 0.f, 0.f));
	m_menuParent->UpdateBounds(AABB2(Vec2(0.f, 0.f), Vec2((float)clientSize.x, (float)clientSize.y)));

	//Create the radio group
	m_menuRadGroup = m_menuParent->CreateChild<UIRadioGroup>(m_menuParent->GetWorldBounds());

	AABB2 bounds = AABB2(Vec2(0.f, 0.f), Vec2(30.f, 30.f));
	Vec4 size = Vec4(0.1f, 0.1f, 0.f, 0.f);
	Vec4 position = Vec4(0.f, 0.f, 0.f, 0.f);
	//Create the Play Button
	m_playButton = m_menuRadGroup->CreateChild<UIButton>(m_menuRadGroup->GetWorldBounds(), size, position);
	m_playButton->SetOnClick("GoToGame");
	m_playButton->SetColor(Rgba::WHITE);
	m_playButton->unHovercolor = Rgba::WHITE;
	m_playButton->SetRadioType(true);

	//Create the Edit button
	bounds = AABB2(Vec2(0.f, 0.f), Vec2(30.f, 30.f));
	size = Vec4(0.1f, 0.1f, 0.f, 0.f);
	position = Vec4(0.f, 0.f, 0.f, 0.f);

	m_editButton = m_menuRadGroup->CreateChild<UIButton>(m_menuRadGroup->GetWorldBounds(), size, position);
	m_editButton->SetOnClick("GoToEdit");
	m_editButton->SetColor(Rgba::WHITE);
	m_editButton->unHovercolor = Rgba::WHITE;
	m_editButton->SetRadioType(true);

	size = Vec4(1.f, .75f, 0.f, 0.f);
	position = Vec4(0.5f, 0.5, 0.f, 0.f);

	UILabel* label = m_playButton->CreateChild<UILabel>(m_playButton->GetWorldBounds(), size, position);
	label->SetLabelText("PLAY");
	label->SetColor(Rgba::WHITE);

	label = m_editButton->CreateChild<UILabel>(m_editButton->GetWorldBounds(), size, position);
	label->SetLabelText("EDIT");
	label->SetColor(Rgba::WHITE);

	//Age of emptiness
	size = Vec4(0.4f, 0.45f, 0.f, 0.f);
	position = Vec4(0.5f, 0.5f, 0.f, -120.f);

	label = m_menuParent->CreateChild<UILabel>(m_menuParent->GetWorldBounds(), size, position);
	label->SetLabelText("Age of Emptiness III");
	label->SetColor(Rgba(0.f, 0.f, 0.f, 1.f));
	
	size = Vec4(0.4f, 0.4f, 0.f, 0.f);
	position = Vec4(0.5f, 0.5f, 0.f, -117.f);

	label = m_menuParent->CreateChild<UILabel>(m_menuParent->GetWorldBounds(), size, position);
	label->SetLabelText("Age of Emptiness III");
	label->SetColor(Rgba::WHITE);

	size = Vec4(0.35f, 0.35f, 0.f, 0.f);
	position = Vec4(0.5f, 0.5f, 0.f, -260.f);

	label = m_menuParent->CreateChild<UILabel>(m_menuParent->GetWorldBounds(), size, position);
	label->SetLabelText("A truly empty 3D experience");
	label->SetColor(Rgba::DARK_GREY);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PerformSingleThreadLoading()
{
	m_carModel = g_renderContext->CreateOrGetMeshFromFile(m_carMeshPath);
	m_wheelModel = g_renderContext->CreateOrGetMeshFromFile(m_wheelMeshPath);
	m_wheelFlippedModel = g_renderContext->CreateOrGetMeshFromFile(m_wheelFlippedMeshPath);
	m_treeModel = g_renderContext->CreateOrGetMeshFromFile(m_treeMeshPath);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadTrackMeshesOnSceneCreation()
{
	m_trackTestModel = g_renderContext->CreateOrGetMeshFromFile(m_trackTestPath);
	m_trackCollidersTestModel = g_renderContext->CreateOrGetMeshFromFile(m_trackCollisionsTestPath);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PerformAsyncLoading()
{
	if (!m_threadedLoadComplete)
	{
		EnqueueLoadingModel(m_carMeshPath);
		EnqueueLoadingModel(m_wheelMeshPath);
		EnqueueLoadingModel(m_wheelFlippedMeshPath);
		EnqueueLoadingModel(m_trackTestPath);
		EnqueueLoadingModel(m_trackCollisionsTestPath);

		int coreCount = std::thread::hardware_concurrency();
		int halfCores = coreCount / 2;
		for (int i = 0; i < halfCores; ++i)
		{
			m_threads.emplace_back(&Game::LoadModelsFromThread, this);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::EnqueueLoadingModel(std::string fileName)
{
	ModelLoadWork* work = new ModelLoadWork(fileName);
	work->modelName = fileName;

	++m_modelLoading;
	m_modelLoadQueue.EnqueueLocked(work);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadModelsFromThread()
{
	ModelLoadWork* work;

	while (m_modelLoadQueue.DequeueLocked(&work))
	{
		//Load the file and make the CPUMesh here
		std::string filePath = MODEL_PATH + work->modelName;
		ObjectLoader object;
		object.m_renderContext = g_renderContext;
		object.LoadFromXML(filePath.c_str());
		work->mesh = object.m_cpuMesh;
		work->materialPath = object.m_defaultMaterialPath;

		m_modelFinishedQueue.EnqueueLocked(work);
	}
	Sleep(0);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::FinishReadyModels()
{
	ModelLoadWork* work;
	while (m_modelFinishedQueue.DequeueLocked(&work))
	{
		//Create the Model with GPUMesh here
		work->model = new Model();
		work->model->m_context = g_renderContext;
		work->model->m_mesh = new GPUMesh(g_renderContext);
		work->model->m_mesh->CreateFromCPUMesh<Vertex_Lit>(work->mesh);
		work->model->m_mesh->m_defaultMaterial = work->materialPath;

		std::string modelPath = work->modelName;
		std::string materialPath = work->materialPath;
		std::vector<std::string> splits = SplitStringOnDelimiter(modelPath, '.');
		if (splits[splits.size() - 1] == "obj" || splits[splits.size() - 1] == "mesh")
		{
			std::vector<std::string> materialSplits = SplitStringOnDelimiter(materialPath, '.');
			materialPath = MODEL_PATH + materialSplits[0] + ".mat";
		}

		if (work->model->m_mesh->m_defaultMaterial != "")
		{
			work->model->m_material = g_renderContext->CreateOrGetMaterialFromFile(materialPath);
		}

		--m_modelLoading;

		delete work;

		ASSERT_RECOVERABLE(m_modelLoading >= 0, "m_modelLoading is less than 0");
	}
}

//------------------------------------------------------------------------------------------------------------------------------
bool Game::IsFinishedModelLoading() const
{
	return (m_modelLoading == 0);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::ImageLoadThread()
{
	ImageLoadWork* work;

	while (m_loadQueue.DequeueLocked(&work))
	{
		//We use our image default constructor to load
		work->image = new Image(work->imageName.c_str());
		m_finishedQueue.EnqueueLocked(work);
	}
	Sleep(0);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::StartLoadingImage(std::string fileName)
{
	ImageLoadWork* work = new ImageLoadWork(fileName);
	work->imageName = fileName;

	++m_imageLoading;
	m_loadQueue.EnqueueLocked(work);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::FinishReadyImages()
{
	ImageLoadWork* work;
	if (m_finishedQueue.DequeueLocked(&work))
	{
		std::string name = work->imageName.c_str();

		Texture2D *texture = new Texture2D(g_renderContext);
		TextureView* textureView = nullptr;
		texture->LoadTextureFromImage(*work->image);
		textureView = texture->CreateTextureView2D();

		g_renderContext->RegisterTextureView(work->imageName, textureView);

		delete work->image;
		work->image = nullptr;
		delete work;
		delete texture;

		--m_imageLoading;

		ASSERT_RECOVERABLE(m_imageLoading >= 0, "m_imageLoading is less than 0");
	}
}

//------------------------------------------------------------------------------------------------------------------------------
bool Game::IsFinishedImageLoading() const
{
	return (m_imageLoading == 0);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateAllCars(float deltaTime)
{
	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		m_cars[carIndex]->Update(deltaTime, m_isXInputEnabled);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CheckForRaceCompletion()
{
	bool raceEnded = true;
	double bestTime = 9999.0;
	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		if (!m_cars[carIndex]->GetWaypointsEditable().AreLapsComplete())
		{
			raceEnded = false;
		}
		else
		{
			if (bestTime > m_cars[carIndex]->GetRaceTime())
			{
				bestTime = m_cars[carIndex]->GetRaceTime();
			}
		}
	}

	if (raceEnded)
	{
		//The race has completed so you can do some logic here
		m_bestTimeForRun = bestTime;

		if (m_bestTimeForRun < m_bestTimeFromFile)
		{
			m_previousBestTime = m_bestTimeFromFile;
			WriteNewBestTimeText();
		}

		HandleRaceCompletedCondition();
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CopyAudioIDsFromFirstCar(int carIndex)
{
	m_cars[carIndex]->GetCarAudioEditable()->SetSimplexSoundID(m_cars[0]->GetCarAudio().GetSimplexSoundID());
	m_cars[carIndex]->GetCarAudioEditable()->SetSimplexSoundPlaybackID(m_cars[0]->GetCarAudio().GetSimplexSoundPlaybackID());

	int size = 0;
	m_cars[carIndex]->GetCarAudioEditable()->SetSoundIDs(m_cars[0]->GetCarAudio().GetSoundIDs(size), size);
	m_cars[carIndex]->GetCarAudioEditable()->SetSoundPlaybackIDs(m_cars[0]->GetCarAudio().GetSoundPlaybackIDs(size), size);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreatePhysXVehicleBoxWall()
{
	//Add a wall made of dynamic objects with cuboid shapes for bricks.
	PxTransform t(PxVec3(-20.f, 0.f, 0.f), PxQuat(-0.000002f, -0.837118f, -0.000004f, 0.547022f));
	CreateObstacleWall(12, 4, 1.0f, t.p, t.q);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreateObstacleWall(const int numHorizontalBoxes, const int numVerticalBoxes, const float boxSize, const PxVec3& pos, const PxQuat& quat)
{
	const PxF32 density = 50.0f;

	const PxF32 sizeX = boxSize;
	const PxF32 sizeY = boxSize;
	const PxF32 sizeZ = boxSize;

	const PxF32 mass = sizeX * sizeY*sizeZ*density;
	const PxVec3 halfExtents(sizeX*0.5f, sizeY*0.5f, sizeZ*0.5f);
	PxBoxGeometry geometry(halfExtents);
	PxTransform shapeTransforms[1] = { PxTransform(PxIdentity) };
	PxGeometry* shapeGeometries[1] = { &geometry };
	PxMaterial* shapeMaterials[1] = { g_PxPhysXSystem->GetDefaultPxMaterial() };

	const PxF32 spacing = 0.0001f;
	PxVec3 relPos(0.0f, sizeY / 2, 0.0f);
	PxF32 offsetX = -(numHorizontalBoxes * (sizeX + spacing) * 0.5f);
	PxF32 offsetZ = 0.0f;

	for (PxU32 k = 0; k < (PxU32)numVerticalBoxes; k++)
	{
		for (PxU32 i = 0; i < (PxU32)numHorizontalBoxes; i++)
		{
			relPos.x = offsetX + (sizeX + spacing)*i;
			relPos.z = offsetZ;
			PxTransform transform(pos + quat.rotate(relPos), quat);
			g_PxPhysXSystem->AddDynamicObstacle(transform, mass, 1, shapeTransforms, shapeGeometries, shapeMaterials);
		}

		if (0 == (k % 2))
		{
			offsetX += sizeX / 2;
		}
		else
		{
			offsetX -= sizeX / 2;
		}
		relPos.y += (sizeY + spacing);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreatePhysXVehicleRamp()
{
	PxPhysics* physX = g_PxPhysXSystem->GetPhysXSDK();
	PxCooking* pxCooking = g_PxPhysXSystem->GetPhysXCookingModule();
	PxMaterial* pxMaterial = g_PxPhysXSystem->GetDefaultPxMaterial();

	//Add a really big ramp to jump over 
	{
		PxVec3 halfExtentsRamp(5.0f, 1.9f, 7.0f);
		PxConvexMeshGeometry geomRamp(g_PxPhysXSystem->CreateWedgeConvexMesh(halfExtentsRamp, *physX, *pxCooking));
		PxTransform shapeTransforms[1] = { PxTransform(PxIdentity) };
		PxMaterial* shapeMaterials[1] = { pxMaterial };
		PxGeometry* shapeGeometries[1] = { &geomRamp };

		Matrix44 bigRampModel;
		bigRampModel.MakeTranslation3D(Vec3(-10.f, 0.f, 0.f));
		Matrix44 rotation = Matrix44::MakeYRotationDegrees(180.f);
		bigRampModel = bigRampModel.AppendMatrix(rotation);

		PxTransform tRamp(g_PxPhysXSystem->VecToPxVector(bigRampModel.GetTBasis()), g_PxPhysXSystem->MakeQuaternionFromMatrix(bigRampModel) );
		g_PxPhysXSystem->AddStaticObstacle(tRamp, 1, shapeTransforms, shapeGeometries, shapeMaterials);
	}

	//Add two ramps side by side somewhere
	{
		PxVec3 halfExtents(3.0f, 1.5f, 3.5f);
		PxConvexMeshGeometry geometry(g_PxPhysXSystem->CreateWedgeConvexMesh(halfExtents, *physX, *pxCooking));
		PxTransform shapeTransforms[1] = { PxTransform(PxIdentity) };
		PxMaterial* shapeMaterials[1] = { pxMaterial };
		PxGeometry* shapeGeometries[1] = { &geometry };
		PxTransform t1(PxVec3(-60.f, 0.f, 0.f), PxQuat(0.000013f, -0.406322f, 0.000006f, 0.913730f));
		g_PxPhysXSystem->AddStaticObstacle(t1, 1, shapeTransforms, shapeGeometries, shapeMaterials);
		PxTransform t2(PxVec3(-80, 0.f, 0.f), PxQuat(0.000013f, -0.406322f, 0.000006f, 0.913730f));
		g_PxPhysXSystem->AddStaticObstacle(t2, 1, shapeTransforms, shapeGeometries, shapeMaterials);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreatePhysXVehicleObstacles()
{
	PxPhysics* physX = g_PxPhysXSystem->GetPhysXSDK();
	PxScene* pxScene = g_PxPhysXSystem->GetPhysXScene();

	PxMaterial* pxMat;
	pxMat = g_PxPhysXSystem->GetDefaultPxMaterial();

	const float boxHalfHeight = 1.0f;
	const float boxZ = 30.0f;
	PxTransform t(PxVec3(0.f, boxHalfHeight, boxZ), PxQuat(PxIdentity));
	PxRigidStatic* rs = physX->createRigidStatic(t);

	PxBoxGeometry boxGeom(PxVec3(3.0f, boxHalfHeight, 3.0f));
	PxShape* shape = PxRigidActorExt::createExclusiveShape(*rs, boxGeom, *pxMat);

	PxFilterData simFilterData(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_WHEEL, PxPairFlag::eMODIFY_CONTACTS | PxPairFlag::eDETECT_CCD_CONTACT, 0);
	shape->setSimulationFilterData(simFilterData);
	PxFilterData qryFilterData;
	setupDrivableSurface(qryFilterData);
	shape->setQueryFilterData(qryFilterData);

	pxScene->addActor(*rs);

	for (PxU32 i = 0; i < 64; i++)
	{
		t = PxTransform(PxVec3(20.f + i * 0.01f, 2.0f + i * 0.25f, 20.0f + i * 0.025f), PxQuat(PxPi*0.5f, PxVec3(0, 1, 0)));
		PxRigidDynamic* rd = physX->createRigidDynamic(t);

		boxGeom = PxBoxGeometry(PxVec3(0.08f, 0.25f, 1.0f));
		shape = PxRigidActorExt::createExclusiveShape(*rd, boxGeom, *pxMat);

		simFilterData = PxFilterData(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, PxPairFlag::eMODIFY_CONTACTS | PxPairFlag::eDETECT_CCD_CONTACT, 0);
		shape->setSimulationFilterData(simFilterData);
		setupDrivableSurface(qryFilterData);
		shape->setQueryFilterData(qryFilterData);

		PxRigidBodyExt::updateMassAndInertia(*rd, 30.0f);

		pxScene->addActor(*rd);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
physx::PxRigidActor* Game::GetCarActor() const
{
	return m_cars[0]->GetCarRigidbody();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreatePhysXConvexHull()
{
	std::vector<Vec3> vertexArray;

	const int numVerts = 64;

	// Prepare random verts
	for (PxU32 i = 0; i < numVerts; i++)
	{
		vertexArray.push_back(Vec3(g_RNG->GetRandomFloatInRange(-5.f, 5.f) , g_RNG->GetRandomFloatInRange(0.f, 5.f), g_RNG->GetRandomFloatInRange(-5.f, 5.f)));
	}

	g_PxPhysXSystem->CreateRandomConvexHull(vertexArray, 16, false);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreatePhysXStack(const Vec3& position, uint size, float halfExtent)
{
	PxPhysics* physX = g_PxPhysXSystem->GetPhysXSDK();
	PxScene* pxScene = g_PxPhysXSystem->GetPhysXScene();

	PxTransform pxTransform = PxTransform(PxVec3(position.x, position.y, position.z));

	//We are going to make a stack of boxes
	PxBoxGeometry box = PxBoxGeometry((PxReal)halfExtent, (PxReal)halfExtent, (PxReal)halfExtent);
	PxMaterial* pxMaterial = physX->createMaterial(0.5f, 0.5f, 0.6f);
	PxShape* shape = physX->createShape(box, *pxMaterial);
	
	//Loop to stack everything in a pyramid shape
	for (PxU32 layerIndex = 0; layerIndex < size; layerIndex++)
	{
		for (PxU32 indexInLayer = 0; indexInLayer < size - layerIndex; indexInLayer++)
		{
			PxTransform localTm(PxVec3(PxReal(indexInLayer * 2) - PxReal(size - layerIndex), PxReal(layerIndex * 2 + 1), 0) * halfExtent);
			PxRigidDynamic* body = physX->createRigidDynamic(pxTransform.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			pxScene->addActor(*body);
		}
	}

	//release the shape now, we don't need it anymore since everything has been added to the PhysX scene
	shape->release();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::ResetCarPositionForPlayer(int playerID)
{
	m_cars[playerID]->ResetCarPosition();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleKeyPressed(unsigned char keyCode)
{
	if(g_devConsole->IsOpen())
	{
		g_devConsole->HandleKeyDown(keyCode);
		return;
	}

	/*
	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		m_cars[carIndex]->GetCarControllerEditable()->HandleKeyPressed(keyCode);
	}
	return;
	*/

	switch( keyCode )
	{
		case UP_ARROW:
		{
			//Increase emissive factor
			m_emissiveFactor += m_emissiveStep;
		}
		break;
		case DOWN_ARROW:
		{
			//decrease emissive factor
			m_emissiveFactor -= m_emissiveStep;
		}
		break;
		case RIGHT_ARROW:
		case LEFT_ARROW:
		case SPACE_KEY:
		{
			/*
			Vec3 velocity;// = m_mainCamera->GetModelMatrix().GetKVector();
			
			velocity = m_mainCamera->GetCameraForward() * 100.f;

			g_PxPhysXSystem->CreateDynamicObject(PxSphereGeometry(3.f), velocity, m_mainCamera->GetModelMatrix(), m_dynamicObjectDensity);
			*/
		}
		break;
		case F1_KEY:
		{
			ResetCarPositionForPlayer(0);
		}
		break;
		case F2_KEY:
		{
			ResetCarPositionForPlayer(1);
		}
		break;
		case F3_KEY:
		{
			ResetCarPositionForPlayer(2);
		}
		break;
		case F4_KEY:
		{
			ResetCarPositionForPlayer(3);
		}
		case A_KEY:
		{
			//Handle left movement
			Vec3 worldMovementDirection = m_mainCamera->m_cameraModel.GetIBasis() * -1.f;
			worldMovementDirection *= (m_cameraSpeed);

			m_camPosition += worldMovementDirection; 
		}
		break;
		case W_KEY:
		{
			//Handle forward movement
			Vec3 worldMovementDirection = m_mainCamera->m_cameraModel.GetKBasis();
			worldMovementDirection *= (m_cameraSpeed); 

			m_camPosition += worldMovementDirection; 
		}
		break;
		case S_KEY:
		{
			//Handle backward movement
			Vec3 worldMovementDirection = m_mainCamera->m_cameraModel.GetKBasis() * -1.f;
			worldMovementDirection *= (m_cameraSpeed); 

			m_camPosition += worldMovementDirection; 
		}
		break;
		case D_KEY:
		{
			//Handle right movement
			Vec3 worldMovementDirection = m_mainCamera->m_cameraModel.GetIBasis();
			worldMovementDirection *= (m_cameraSpeed); 

			m_camPosition += worldMovementDirection; 
		}
		break;
		case F5_KEY:
		{
			//Set volume to 0
			//g_audio->SetSoundPlaybackVolume(m_testPlayback, 0.0f);
			
			//toggle material or not
			m_useMaterial = !m_useMaterial;

			break;
		}
		case F6_KEY:
		{
			//Fire event
			g_eventSystem->FireEvent("TestEvent");
			break;
		}		case F7_KEY:
		{
			//Quit Debug
			g_eventSystem->FireEvent("Quit");
			break;
		}
		case NUM_1:
		{
			m_debugViewCarCollider = !m_debugViewCarCollider;
			break;
		}
		case NUM_2:
		{
			m_debugPerfEnabled = !m_debugPerfEnabled;
			break;
		}
		case NUM_3:
		{
			m_enableImGUI = !m_enableImGUI;
			break;
		}
		case ENTER_KEY:
		{
			if (!m_initiateFromMenu)
			{
				m_initiateFromMenu = true;
				InitiateGameSequence();
			}
		}
		case R_KEY:
		{
			m_isRaceCompleted = false;
			RestartLevel();

			if (m_isRaceCompleted)
			{
				
			}
		}
		break;
		default:
		break;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::DebugEnabled()
{
	g_debugMode = !g_debugMode;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	//m_carController->ReleaseVehicle();

	DeleteUI();

	for (int i = 0; i < m_numConnectedPlayers; i++)
	{
		m_cars[i]->Shutdown();
		delete m_cars[i];
		m_cars[i] = nullptr;
	}

	delete m_UICamera;
	m_UICamera = nullptr;

	delete m_mainCamera;
	m_mainCamera = nullptr;

	delete m_devConsoleCamera;
	m_devConsoleCamera = nullptr;

	TODO("DEBUG: m_baseQuad causes a memory leak");
	delete m_baseQuad;
	m_baseQuad = nullptr;

	delete m_pxCube;
	m_pxCube = nullptr;

	delete m_pxSphere;
	m_pxSphere = nullptr;

	delete m_pxConvexMesh;
	m_pxConvexMesh = nullptr;

	delete m_pxCapMesh;
	m_pxCapMesh = nullptr;

// 	delete m_carModel;
// 	m_carModel = nullptr;
// 
// 	delete m_wheelModel;
// 	m_wheelModel = nullptr;
// 
// 	delete m_wheelFlippedModel;
// 	m_wheelFlippedModel = nullptr;
// 
// 	delete m_trackPieceModel;
// 	m_trackPieceModel = nullptr;

	//FreeResources();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleKeyReleased(unsigned char keyCode)
{
	if(g_devConsole->IsOpen())
	{
		g_devConsole->HandleKeyUp(keyCode);
		return;
	}

	//SoundID testSound = g_audio->CreateOrGetSound( "Data/Audio/TestSound.mp3" );
	switch( keyCode )
	{
		case UP_ARROW:
		case RIGHT_ARROW:
		case LEFT_ARROW:
		//g_audio->PlaySound( m_testAudioID );
		break;
		default:
		break;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleCharacter( unsigned char charCode )
{
	if(g_devConsole->IsOpen())
	{
		g_devConsole->HandleCharacter(charCode);
		return;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
bool Game::HandleMouseScroll(float wheelDelta)
{
	m_frameZoomDelta -= wheelDelta;

	m_frameZoomDelta = Clamp(m_frameZoomDelta, MIN_ZOOM_STEPS, MAX_ZOOM_STEPS);

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::Render() const
{	
	//Set all the frame color targets for the cameras being used
	SetFrameColorTargetOnCameras();

	// Move the camera to where it is in the scene
	Matrix44 camTransform = Matrix44::MakeFromEuler( m_mainCamera->GetEuler(), m_rotationOrder ); 
	camTransform = Matrix44::SetTranslation3D(m_camPosition, camTransform);
	m_mainCamera->SetModelMatrix(camTransform);
	
	SetAmbientIntensity();
	SetEmissiveIntensity();

	if (!m_enableDirectional)
	{
		g_renderContext->DisableDirectionalLight();
	}
	else
	{
		g_renderContext->EnableDirectionalLight();
	}

	//Should I render the menu screen?
	if (!m_initiateFromMenu)
	{
		RenderMenuScreen();
		return;
	}

	//Should I just return since my assets are not loaded
	if (!m_threadedLoadComplete)
	{
		return;
	}

	if (ui_swapToMainCamera)
	{
		RenderScreenForMainCamera();
	}
	else
	{
		RenderSceneForCarCameras();
	}

	//Perform all UI Draws
	m_UICamera->SetViewport(Vec2::ZERO, Vec2::ONE);
	m_UICamera->SetModelMatrix(Matrix44::IDENTITY);

	g_renderContext->BeginCamera(*m_UICamera);
	g_renderContext->BindShader(m_shader);

	if (m_debugPerfEnabled)
	{
		RenderDebugInfoOnScreen();
	}

	g_renderContext->EndCamera();

	if (m_isRaceCompleted)
	{
		RenderRaceCompleted();
	}

	if (g_devConsole->IsOpen())
	{
		g_devConsole->Render(*g_renderContext, *m_devConsoleCamera, DEVCONSOLE_LINE_HEIGHT);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderRacetrack() const
{
	std::string matName = m_trackTestModel->GetDefaultMaterialName();
	g_renderContext->BindMaterial(g_renderContext->CreateOrGetMaterialFromFile(matName));
	//g_renderContext->BindTextureViewWithSampler(0U, m_textureTest);
	g_renderContext->SetModelMatrix(m_trackTestTransform);
	g_renderContext->DrawMesh(m_trackTestModel);

	g_renderContext->BindMaterial( m_defaultMaterial);
	g_renderContext->SetModelMatrix(m_trackTestTransform);
	g_renderContext->DrawMesh(m_trackCollidersTestModel);

	g_renderContext->BindMaterial(g_renderContext->CreateOrGetMaterialFromFile(m_treeModel->GetDefaultMaterialName()));
	g_renderContext->SetModelMatrix(m_treeTransform);
	g_renderContext->DrawMesh(m_treeModel);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderUsingMaterial() const
{
	g_renderContext->BindMaterial(m_couchMaterial);

	//Render the base quad
	g_renderContext->SetModelMatrix(m_baseQuadTransform);
	g_renderContext->DrawMesh(m_baseQuad);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderPhysXScene() const
{
	//Get the actors in the scene
	PxScene* scene;
	PxGetPhysics().getScenes(&scene, 1);

	//Bind Material
	g_renderContext->BindMaterial(m_defaultMaterial);

	int numActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
	if (numActors > 0)
	{
		std::vector<PxRigidActor*> actors(numActors);
		scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), numActors);

		Rgba color = Rgba(0.f, 0.4f, 0.f, 1.f);
		RenderPhysXActors(actors, (int)actors.size(), color);
	}

	int numArticulations = scene->getNbArticulations();
	if (numArticulations > 0)
	{
		PxArticulationBase* articulation;
		scene->getArticulations(&articulation, 1);

		int numLinks = articulation->getNbLinks();
		std::vector<PxArticulationLink*> links(numLinks);
		articulation->getLinks(&links[0], numLinks);

		std::vector<PxRigidActor*> actors;
		for (int i = 0; i < numLinks; ++i)
		{
			actors.push_back(reinterpret_cast<PxRigidActor*>(links[i]));
		}

		Rgba color = Rgba(1.f, 1.f, 1.f, 1.f);
		RenderPhysXActors(actors, (int)actors.size(), color);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderPhysXCar(const CarController& carController) const
{
	//Draw a maximum of 10 shapes
	PxShape* shapes[10] = { nullptr };
	CPUMesh carMesh;
	CPUMesh cvxMesh;
	Matrix44 model;

	PxRigidActor *car = carController.GetVehicle()->getRigidDynamicActor();
	int numShapes = car->getNbShapes();
	car->getShapes(shapes, numShapes);

	//The car and wheel use the same material so only need to bind this once
	g_renderContext->BindMaterial(g_renderContext->CreateOrGetMaterialFromFile(m_wheelModel->GetDefaultMaterialName()));

	for (int shapeIndex = 0; shapeIndex < numShapes; shapeIndex++)
	{
		PxConvexMeshGeometry geometry;
		shapes[shapeIndex]->getConvexMeshGeometry(geometry);

		PxMat44 pxMat = car->getGlobalPose() * shapes[shapeIndex]->getLocalPose();

		model.SetIBasis(g_PxPhysXSystem->PxVectorToVec(pxMat.column0));
		model.SetJBasis(g_PxPhysXSystem->PxVectorToVec(pxMat.column1));
		model.SetKBasis(g_PxPhysXSystem->PxVectorToVec(pxMat.column2));
		model.SetTBasis(g_PxPhysXSystem->PxVectorToVec(pxMat.column3));

		if (geometry.convexMesh->getNbVertices() == 8)
		{
			//This is the car because we know the car mesh is basically a box (8 verts)
			Vec4 forwardOffsetVec4 = model.GetKBasis4() * 0.3f;
			model.SetTBasis(g_PxPhysXSystem->PxVectorToVec(pxMat.column3) + m_offsetCarBody + forwardOffsetVec4);

			g_renderContext->SetModelMatrix(model);

			//Draw the car mesh
			g_renderContext->SetModelMatrix(model);
			g_renderContext->DrawMesh(m_carModel);

			if (m_debugViewCarCollider)
			{
				Matrix44 modelMatrix = Matrix44::IDENTITY;
				//modelMatrix.SetTranslation3D(Vec3(0.f, 100.f, 0.f), modelMatrix);
				g_renderContext->SetModelMatrix(modelMatrix);
				g_renderContext->BindMaterial(m_defaultMaterial);
				AddMeshForConvexMesh(cvxMesh, *car, *shapes[shapeIndex], Rgba::MAGENTA);
				GPUMesh debugMesh(g_renderContext);
				debugMesh.CreateFromCPUMesh<Vertex_Lit>(&cvxMesh);
				g_renderContext->DrawMesh(&debugMesh);
			}
		
			Rgba color = Rgba(1.f, 1.f, 1.f, 1.f);
			int numCarShapes = car->getNbShapes();;
			std::vector<PxShape*> carShapes(numCarShapes);
		}
		else
		{
			g_renderContext->SetModelMatrix(model);
			if (shapeIndex == 1 || shapeIndex == 3)
			{
				g_renderContext->DrawMesh(m_wheelFlippedModel);
			}
			else
			{
				g_renderContext->DrawMesh(m_wheelModel);
			}

			if (m_debugViewCarCollider)
			{	
				Matrix44 modelMatrix = Matrix44::IDENTITY;
				g_renderContext->SetModelMatrix(modelMatrix);
				AddMeshForConvexMesh(cvxMesh, *car, *shapes[shapeIndex], Rgba::MAGENTA);
				GPUMesh debugMesh(g_renderContext);
				debugMesh.CreateFromCPUMesh<Vertex_Lit>(&cvxMesh);
				g_renderContext->DrawMesh(&debugMesh);
			}

		}
	}

	
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderPhysXActors(const std::vector<PxRigidActor*> actors, int numActors, Rgba& color) const
{
	//Look for maximum of 10 shapes to draw per actor
	PxShape* shapes[10] = { nullptr };

	CPUMesh boxMesh;
	CPUMesh sphereMesh;
	CPUMesh cvxMesh;
	CPUMesh capMesh;

	for (int actorIndex = 0; actorIndex < numActors; actorIndex++)
	{
		const int numShapes = actors[actorIndex]->getNbShapes();
		actors[actorIndex]->getShapes(shapes, numShapes);

		const bool sleeping = actors[actorIndex]->is<PxRigidDynamic>() ? actors[actorIndex]->is<PxRigidDynamic>()->isSleeping() : false;

		for (int shapeIndex = 0; shapeIndex < numShapes; shapeIndex++)
		{
			int type = shapes[shapeIndex]->getGeometryType();

			switch (type)
			{
			case PxGeometryType::eBOX:
			{
				color = GetColorForGeometry(type, sleeping);
				AddMeshForPxCube(boxMesh, *actors[actorIndex], *shapes[shapeIndex], color);
			}
			break;
			case PxGeometryType::eSPHERE:
			{
				color = GetColorForGeometry(type, sleeping);
				AddMeshForPxSphere(sphereMesh, *actors[actorIndex], *shapes[shapeIndex], color);
			}
			break;
			case PxGeometryType::eCONVEXMESH:
			{
				/*
				if (ui_enableConvexHullRenders)
				{
					//I don't want to render anything that belongs to the car rigidActor
					//This way I render all PhysX elements except the car here and can use
					//the RenderPhysXCar function instead for the car render
					if (actors[actorIndex] != m_carController->GetVehicle()->getRigidDynamicActor() || actors[actorIndex] != m_player2CarController->GetVehicle()->getRigidDynamicActor())
					{
						color = GetColorForGeometry(type, sleeping);
						AddMeshForConvexMesh(cvxMesh, *actors[actorIndex], *shapes[shapeIndex], color);
					}
				}

				if (ui_enableCarDebug)
				{
					//We want to debug render the car here instead of above 
					if (actors[actorIndex] == m_carController->GetVehicle()->getRigidDynamicActor() || actors[actorIndex] != m_player2CarController->GetVehicle()->getRigidDynamicActor())
					{
						color = GetColorForGeometry(type, sleeping);
						AddMeshForConvexMesh(cvxMesh, *actors[actorIndex], *shapes[shapeIndex], color);
					}
				}
				*/
			}
			break;
			case PxGeometryType::eCAPSULE:
			{
				color = GetColorForGeometry(type, sleeping);
				AddMeshForPxCapsule(capMesh, *actors[actorIndex], *shapes[shapeIndex], color);
			}
			break;
			default:
				break;
			}

		}
	}

	g_renderContext->SetModelMatrix(Matrix44::IDENTITY);
	
	if (boxMesh.GetVertexCount() > 0)
	{
		m_pxCube->CreateFromCPUMesh<Vertex_Lit>(&boxMesh, GPU_MEMORY_USAGE_STATIC);
		g_renderContext->DrawMesh(m_pxCube);
	}

	if (sphereMesh.GetVertexCount() > 0)
	{
		m_pxSphere->CreateFromCPUMesh<Vertex_Lit>(&sphereMesh, GPU_MEMORY_USAGE_STATIC);

		g_renderContext->BindShader(m_shader);
		g_renderContext->BindTextureViewWithSampler(0U, m_sphereTexture);
		g_renderContext->DrawMesh(m_pxSphere);
	}

	if (cvxMesh.GetVertexCount() > 0)
	{
		m_pxConvexMesh->CreateFromCPUMesh<Vertex_Lit>(&cvxMesh, GPU_MEMORY_USAGE_STATIC);
		g_renderContext->DrawMesh(m_pxConvexMesh);
	}

	if (capMesh.GetVertexCount() > 0)
	{
		m_pxCapMesh->CreateFromCPUMesh<Vertex_Lit>(&capMesh, GPU_MEMORY_USAGE_STATIC);
		g_renderContext->DrawMesh(m_pxCapMesh);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderMainMenu() const
{
	IntVec2 clientSize = g_windowContext->GetTrueClientBounds();

	//Render the Menu UI 
	m_menuParent->UpdateBounds(AABB2(Vec2(0.f, 0.f), Vec2((float)clientSize.x, (float)clientSize.y)));

	m_menuParent->Render();	
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderMenuScreen() const
{
	m_UICamera->SetViewport(Vec2::ZERO, Vec2::ONE);
	m_UICamera->SetModelMatrix(Matrix44::IDENTITY);

	g_renderContext->BeginCamera(*m_UICamera);
	g_renderContext->BindShader(m_shader);

	g_renderContext->BindTextureViewWithSampler(0U, nullptr);

	Vec2 camMinBounds = m_UICamera->GetOrthoBottomLeft();
	Vec2 camMaxBounds = m_UICamera->GetOrthoTopRight();

	//Draw a background image
	std::vector<Vertex_PCU> backgroundImageVerts;
	g_renderContext->BindTextureViewWithSampler(0U, nullptr);
	AddVertsForAABB2D(backgroundImageVerts, AABB2(camMinBounds, camMaxBounds), Rgba::BLACK);
	g_renderContext->DrawVertexArray(backgroundImageVerts);

	//Draw a join message
	std::string printString = "Press RETURN to start game!";
	Vec2 textBoxOffset = Vec2(400.f, 25.f);
	std::vector<Vertex_PCU> textVerts;
	m_menuFont->AddVertsForTextInBox2D(textVerts, AABB2(camMaxBounds * 0.5f - textBoxOffset , camMaxBounds * 0.5f + textBoxOffset), m_menuFontHeight, printString, Rgba::ORGANIC_YELLOW);
	//m_squirrelFont->AddVertsForText2D(textVerts, Vec2(0.f, 0.f), m_fontHeight, printString, Rgba::RED);

	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);
	g_renderContext->DrawVertexArray(textVerts);

	g_renderContext->EndCamera();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderRaceCompleted() const
{
	m_UICamera->SetViewport(Vec2::ZERO, Vec2::ONE);
	m_UICamera->SetModelMatrix(Matrix44::IDENTITY);

	g_renderContext->BeginCamera(*m_UICamera);
	g_renderContext->BindShader(m_shader);

	g_renderContext->BindTextureViewWithSampler(0U, nullptr);

	Vec2 camMinBounds = m_UICamera->GetOrthoBottomLeft();
	Vec2 camMaxBounds = m_UICamera->GetOrthoTopRight();

	//Draw a screen Overlay
	std::vector<Vertex_PCU> backgroundImageVerts;
	g_renderContext->BindTextureViewWithSampler(0U, nullptr);	
	AddVertsForAABB2D(backgroundImageVerts, AABB2(camMinBounds, camMaxBounds), Rgba(0.f, 0.f, 0.f, 0.45f));
	g_renderContext->DrawVertexArray(backgroundImageVerts);

	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);

	std::vector<Vertex_PCU> timeVerts;
	AddVertsForPlayerTimesInOrder(timeVerts);
	g_renderContext->DrawVertexArray(timeVerts);

	//Draw a join message
	std::string printString = "Press R to Restart the game";
	Vec2 offset = Vec2(camMinBounds.x + 100.f, camMaxBounds.y * 0.5f + 200.f);
	std::vector<Vertex_PCU> textVerts;
	m_menuFont->AddVertsForText2D(textVerts, camMinBounds + offset, m_menuFontHeight, printString, Rgba::ORGANIC_RED);
	//m_squirrelFont->AddVertsForText2D(textVerts, Vec2(0.f, 0.f), m_fontHeight, printString, Rgba::RED);

	g_renderContext->DrawVertexArray(textVerts);

	g_renderContext->EndCamera();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::AddVertsForPlayerTimesInOrder(std::vector<Vertex_PCU>& timeVerts) const
{
	float time[4];
	int indices[4];

	for (int index = 0; index < m_numConnectedPlayers; index++)
	{
		time[index] = (float)m_cars[index]->GetRaceTime();
		indices[index] = index;
	}

	//Sort the list in decending order
	for (int i = 0; i < m_numConnectedPlayers; i++)
	{
		for (int j = i; j < m_numConnectedPlayers; j++)
		{
			if (time[j] < time[i])
			{
				float temp = time[i];
				time[i] = time[j];
				time[j] = temp;

				int tempIndex = indices[i];
				indices[i] = indices[j];
				indices[j] = tempIndex;
			}
		}
	}

	//Now print the player times in decending order:
	AABB2 camBounds = AABB2(m_UICamera->GetOrthoBottomLeft(), m_UICamera->GetOrthoTopRight());
	Vec2 offset = Vec2(camBounds.m_minBounds.x + 100.f, camBounds.m_maxBounds.y * 0.5f);
	std::string printString = "";

	for (int playerIndex = 0; playerIndex < m_numConnectedPlayers; playerIndex++)
	{
		printString = Stringf("Position: %d : Player : %d Time : %.2f", playerIndex + 1, indices[playerIndex] + 1, time[playerIndex]);
		m_menuFont->AddVertsForText2D(timeVerts, camBounds.m_minBounds + offset, m_menuFontHeight, printString, Rgba::ORGANIC_BLUE);
		offset.y -= m_menuFontHeight * 1.5f;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderUITest() const
{
	g_renderContext->BindTextureViewWithSampler(0U, nullptr);

	Vec2 camMinBounds = m_UICamera->GetOrthoBottomLeft();
	Vec2 camMaxBounds = m_UICamera->GetOrthoTopRight();

	//Toggle UI 
	std::string printString = "Toggle Control Scheme (LCTRL Button) ";
	std::vector<Vertex_PCU> textVerts;
	m_menuFont->AddVertsForText2D(textVerts, Vec2(0.f, 0.f), m_fontHeight, printString, Rgba::WHITE);

	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture());
	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderDebugInfoOnScreen() const
{
	Vec2 camMinBounds = m_UICamera->GetOrthoBottomLeft();
	Vec2 camMaxBounds = m_UICamera->GetOrthoTopRight();

	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture(), SAMPLE_MODE_POINT);

	Vec2 displayArea = Vec2::ZERO;
	displayArea.y = camMaxBounds.y - m_fontHeight;

	//Print FPS info (Avg FPS)
	std::string printString = Stringf("FPS: %.2f", m_avgFPS);
	std::vector<Vertex_PCU> textVerts;

	m_menuFont->AddVertsForText2D(textVerts, displayArea, m_fontHeight, printString, Rgba::WHITE);
	g_renderContext->DrawVertexArray(textVerts);

	displayArea.y -= m_fontHeight;

	//Lowest FPS
	printString = Stringf("Lowest FPS: %.2f", m_fpsLowest);
	textVerts.clear();
	m_menuFont->AddVertsForText2D(textVerts, displayArea, m_fontHeight, printString, Rgba::ORGANIC_DIM_RED);
	g_renderContext->DrawVertexArray(textVerts);

	displayArea.y -= m_fontHeight;

	//Highest FPS
	printString = Stringf("Highest FPS: %.2f", m_fpsHighest);
	textVerts.clear();
	m_menuFont->AddVertsForText2D(textVerts, displayArea, m_fontHeight, printString, Rgba::ORGANIC_GREEN);
	g_renderContext->DrawVertexArray(textVerts);

	displayArea.y -= m_fontHeight;

	//(Frame delta)
	printString = Stringf("Frame Delta-Time: %.5f", m_deltaTime);
	textVerts.clear();
	m_menuFont->AddVertsForText2D(textVerts, displayArea, m_fontHeight, printString, Rgba::WHITE);
	g_renderContext->DrawVertexArray(textVerts);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetupCarHUDsFromSplits(eSplitMode splitMode) const
{
	switch (m_numConnectedPlayers)
	{
	case 1:
	{
		//We have only 1 player so we should be fine with the viewport being the whole screen
		m_cars[0]->GetCarHUDCamera().SetViewport(Vec2::ZERO, Vec2::ONE);
	}
	break;
	case 2:
	{
		//We have 2 players, check the split mode for 2P and split accordingly
		if (splitMode == PREFER_VERTICAL_SPLIT)
		{
			//We need to split the screen into vertical halfs
			m_cars[0]->GetCarHUDCamera().SetViewport(SplitScreenSystem::VERTICAL_SPLIT_2P_FIRST_MIN, SplitScreenSystem::VERTICAL_SPLIT_2P_FIRST_MAX);
			m_cars[1]->GetCarHUDCamera().SetViewport(SplitScreenSystem::VERTICAL_SPLIT_2P_SECOND_MIN, SplitScreenSystem::VERTICAL_SPLIT_2P_SECOND_MAX);
		}
		else
		{
			//We need to split the screen into horizontal halfs
			m_cars[0]->GetCarHUDCamera().SetViewport(SplitScreenSystem::HORIZONTAL_SPLIT_2P_FIRST_MIN, SplitScreenSystem::HORIZONTAL_SPLIT_2P_FIRST_MAX);
			m_cars[1]->GetCarHUDCamera().SetViewport(SplitScreenSystem::HORIZONTAL_SPLIT_2P_SECOND_MIN, SplitScreenSystem::HORIZONTAL_SPLIT_2P_SECOND_MAX);
		}
	}
	break;
	case 3:
	{
		if (splitMode == PREFER_VERTICAL_SPLIT)
		{
			m_cars[0]->GetCarHUDCamera().SetViewport(SplitScreenSystem::VERTICAL_SPLIT_3P_FIRST_MIN, SplitScreenSystem::VERTICAL_SPLIT_3P_FIRST_MAX);
			m_cars[1]->GetCarHUDCamera().SetViewport(SplitScreenSystem::VERTICAL_SPLIT_3P_SECOND_MIN, SplitScreenSystem::VERTICAL_SPLIT_3P_SECOND_MAX);
			m_cars[2]->GetCarHUDCamera().SetViewport(SplitScreenSystem::VERTICAL_SPLIT_3P_THIRD_MIN, SplitScreenSystem::VERTICAL_SPLIT_3P_THIRD_MAX);
		}
		else
		{
			m_cars[0]->GetCarHUDCamera().SetViewport(SplitScreenSystem::HORIZONTAL_SPLIT_3P_FIRST_MIN, SplitScreenSystem::HORIZONTAL_SPLIT_3P_FIRST_MAX);
			m_cars[1]->GetCarHUDCamera().SetViewport(SplitScreenSystem::HORIZONTAL_SPLIT_3P_SECOND_MIN, SplitScreenSystem::HORIZONTAL_SPLIT_3P_SECOND_MAX);
			m_cars[2]->GetCarHUDCamera().SetViewport(SplitScreenSystem::HORIZONTAL_SPLIT_3P_THIRD_MIN, SplitScreenSystem::HORIZONTAL_SPLIT_3P_THIRD_MAX);
		}
	}
	break;
	case 4:
	{
		if (splitMode == PREFER_VERTICAL_SPLIT)
		{
			m_cars[0]->GetCarHUDCamera().SetViewport(SplitScreenSystem::VERTICAL_SPLIT_4P_FIRST_MIN, SplitScreenSystem::VERTICAL_SPLIT_4P_FIRST_MAX);
			m_cars[1]->GetCarHUDCamera().SetViewport(SplitScreenSystem::VERTICAL_SPLIT_4P_SECOND_MIN, SplitScreenSystem::VERTICAL_SPLIT_4P_SECOND_MAX);
			m_cars[2]->GetCarHUDCamera().SetViewport(SplitScreenSystem::VERTICAL_SPLIT_4P_THIRD_MIN, SplitScreenSystem::VERTICAL_SPLIT_4P_THIRD_MAX);
			m_cars[3]->GetCarHUDCamera().SetViewport(SplitScreenSystem::VERTICAL_SPLIT_4P_FOURTH_MIN, SplitScreenSystem::VERTICAL_SPLIT_4P_FOURTH_MAX);
		}
		else
		{
			m_cars[0]->GetCarHUDCamera().SetViewport(SplitScreenSystem::HORIZONTAL_SPLIT_4P_FIRST_MIN, SplitScreenSystem::HORIZONTAL_SPLIT_4P_FIRST_MAX);
			m_cars[1]->GetCarHUDCamera().SetViewport(SplitScreenSystem::HORIZONTAL_SPLIT_4P_SECOND_MIN, SplitScreenSystem::HORIZONTAL_SPLIT_4P_SECOND_MAX);
			m_cars[2]->GetCarHUDCamera().SetViewport(SplitScreenSystem::HORIZONTAL_SPLIT_4P_THIRD_MIN, SplitScreenSystem::HORIZONTAL_SPLIT_4P_THIRD_MAX);
			m_cars[3]->GetCarHUDCamera().SetViewport(SplitScreenSystem::HORIZONTAL_SPLIT_4P_FOURTH_MIN, SplitScreenSystem::HORIZONTAL_SPLIT_4P_FOURTH_MAX);
		}
	}
	break;
	default:
		break;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::DeleteUI()
{
	delete m_menuParent;
	m_menuParent = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RestartLevel()
{
	//Set all cars to have no forces acting on them
	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		SetEnableXInput(false);

		CarController* controller = m_cars[carIndex]->GetCarControllerEditable();
		controller->ReleaseAllControls();
		controller->GetVehicle()->getRigidDynamicActor()->clearForce();
		controller->GetVehicle()->getRigidDynamicActor()->clearTorque();

		controller->GetVehicle()->getRigidDynamicActor()->setForceAndTorque(PxVec3(0.f, 0.f, 0.f), PxVec3(0.f, 0.f, 0.f));
		controller->GetVehicle()->getRigidDynamicActor()->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
		controller->GetVehicle()->getRigidDynamicActor()->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));

		//Reset all car data values (Timing and waypoints and what not)
		m_cars[carIndex]->ResetWaypointSystem();

		//Set all cars to their initial positions
		controller->SetVehiclePosition(m_startPositions[carIndex]);
		controller->SetVehicleDefaultOrientation();
		
		SetEnableXInput(true);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetCarHUDColorTargets(ColorTargetView * colorTargetView) const
{
	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		m_cars[carIndex]->GetCarHUDCamera().SetColorTarget(colorTargetView);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::WriteNewBestTime()
{
	//Open the xml file and parse it
	tinyxml2::XMLDocument timeDoc;
	timeDoc.LoadFile(m_saveFilePath.c_str());

	if (timeDoc.ErrorID() != tinyxml2::XML_SUCCESS)
	{

		ERROR_AND_DIE(">> Error save file ");
		return;
	}
	else
	{
// 		timeDoc.Clear();
// 
// 		XMLElement* root = new XMLElement(&timeDoc);
// 		XMLElement* child = new XMLElement(&timeDoc);
// 
// 		std::string saveString = Stringf("time = %f", m_bestTimeForRun);
// 		tinyxml2::XMLText* text = new tinyxml2::XMLText(saveString);
// 
// 		child->LinkEndChild(text);
// 		timeDoc.LinkEndChild(root);
// 		timeDoc.LinkEndChild(child);
// 		timeDoc.SaveFile("madeByHand.xml");
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::WriteNewBestTimeText()
{
	std::ofstream* writeStream = CreateFileWriteBuffer(m_saveFileTextPath);
	
	std::string writeString = std::to_string(m_bestTimeForRun);

	writeStream->write(writeString.c_str(), writeString.length());
	writeStream->flush();

	writeStream->close();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreateBaseBoxForCollisionDetection()
{
	PxPhysics* physX = g_PxPhysXSystem->GetPhysXSDK();
	PxScene* pxScene = g_PxPhysXSystem->GetPhysXScene();

	PxMaterial* pxMat;
	pxMat = g_PxPhysXSystem->GetDefaultPxMaterial();

	const float boxHalfHeight = 0.05f;
	const float boxXZ = 1000.0f;
	PxTransform t(PxVec3(0.f, boxHalfHeight, 0.f), PxQuat(PxIdentity));
	PxRigidStatic* rs = physX->createRigidStatic(t);

	PxBoxGeometry boxGeom(PxVec3(boxXZ, boxHalfHeight, boxXZ));
	PxShape* shape = PxRigidActorExt::createExclusiveShape(*rs, boxGeom, *pxMat);

	PxFilterData simFilterData(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_WHEEL, PxPairFlag::eMODIFY_CONTACTS | PxPairFlag::eDETECT_CCD_CONTACT, 0);
	shape->setSimulationFilterData(simFilterData);
	PxFilterData qryFilterData;
	setupDrivableSurface(qryFilterData);
	shape->setQueryFilterData(qryFilterData);

	pxScene->addActor(*rs);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::ResetCarsUsingToolData()
{
	//Use tool data and re-create car controllers

	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		CarController* controller = m_cars[carIndex]->GetCarControllerEditable();
		//Vec3 carPosition = controller->GetVehiclePosition();

		//Remove actor from scene
		controller->RemoveVehicleFromScene();

		//Make a new car using the data from tool
		controller->SetNewPxVehicle(m_carTool.MakeNewCar());
		//controller->SetVehiclePosition(carPosition);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::ReadBestTimeFromFile()
{
	//Open the xml file and parse it
	tinyxml2::XMLDocument timeDoc;
	timeDoc.LoadFile(m_saveFilePath.c_str());

	if (timeDoc.ErrorID() != tinyxml2::XML_SUCCESS)
	{

		ERROR_AND_DIE(">> Error save file ");
		return;
	}
	else
	{
		//We loaded the file successfully
		XMLElement* root = timeDoc.RootElement();
		XMLElement* elem = root->FirstChildElement("BestTime");

		UNUSED(root);
		UNUSED(elem);

		if (root->FindAttribute("time"))
		{
			m_bestTimeFromFile = ParseXmlAttribute(*root, "time", (float)m_bestTimeFromFile);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::ReadBestTimeFromTextFile()
{
	char* buffer = new char[20];
	//unsigned long size = 
	CreateFileReadBuffer(m_saveFileTextPath, &buffer);
	m_bestTimeFromFile = atof(buffer);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetMeshesAndJoinThreads()
{
	m_carModel = g_renderContext->CreateOrGetMeshFromFile(m_carMeshPath);
	m_wheelModel = g_renderContext->CreateOrGetMeshFromFile(m_wheelMeshPath);
	m_wheelFlippedModel = g_renderContext->CreateOrGetMeshFromFile(m_wheelFlippedMeshPath);

	m_trackTestModel = g_renderContext->CreateOrGetMeshFromFile(m_trackTestPath);
	m_trackCollidersTestModel = g_renderContext->CreateOrGetMeshFromFile(m_trackCollisionsTestPath);

	for (std::thread& threadHandle : m_threads)
	{
		threadHandle.join();
	}

	m_threads.clear();

	m_threadedLoadComplete = true;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleRaceCompletedCondition()
{
	//Disable all user input from Xbox controller (Nice effect where the cars just keep going with their momentum)
	SetEnableXInput(false);
	//Enable the render method for race completion
	m_isRaceCompleted = true;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetEnableXInput(bool isEnabled /*= true*/)
{
	m_isXInputEnabled = isEnabled;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderSceneForCarCameras() const
{
	//For Car Camera view
	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		Car* car = const_cast<Car*>(m_cars[carIndex]);
		g_renderContext->BeginCamera(*car->GetCarCameraEditable());

		if (carIndex == 0)
		{
			//Only clear the color target view the first time
			g_renderContext->ClearColorTargets(*m_clearScreenColor);
		}

		RenderRacetrack();

		//Render the Quad
		g_renderContext->BindMaterial(m_defaultMaterial);
		//g_renderContext->BindTextureViewWithSampler(0U, m_floorTexture);
		g_renderContext->SetModelMatrix(m_baseQuadTransform);
		g_renderContext->DrawMesh(m_baseQuad);

		//g_renderContext->SetModelMatrix(m_cars[carIndex].GetWaypoints().GetNextWaypointModelMatrix());
		g_renderContext->SetModelMatrix(Matrix44::IDENTITY);
		m_cars[carIndex]->GetWaypoints().RenderNextWaypoint();

		for (int renderCarIndex = 0; renderCarIndex < m_numConnectedPlayers; renderCarIndex++)
		{
			RenderPhysXCar(m_cars[renderCarIndex]->GetCarController());
		}

		g_renderContext->EndCamera();

		m_cars[carIndex]->RenderUIHUD();

		//RenderGearNumber(carIndex);
	}
}

//------------------------------------------------------------------------------------------------------------------------------]
void Game::RenderScreenForMainCamera() const
{
	//For regular PhysX camera
	g_renderContext->BeginCamera(*m_mainCamera);

	RenderRacetrack();

	//Render the Quad
	g_renderContext->BindMaterial(m_defaultMaterial);
	g_renderContext->BindTextureViewWithSampler(0U, nullptr);
	g_renderContext->SetModelMatrix(m_baseQuadTransform);
	g_renderContext->DrawMesh(m_baseQuad);


	for (int renderCarIndex = 0; renderCarIndex < m_numConnectedPlayers; renderCarIndex++)
	{
		RenderPhysXCar(m_cars[renderCarIndex]->GetCarController());
	}

	g_renderContext->EndCamera();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetAmbientIntensity() const
{
	float intensity = Clamp(m_ambientIntensity, 0.f, 1.f);
	g_renderContext->SetAmbientLight(Rgba::WHITE, intensity);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetEmissiveIntensity() const
{
	float emissive = Clamp(m_emissiveFactor, 0.1f, 1.f);
	g_renderContext->m_cpuLightBuffer.emissiveFactor = emissive;
}

//------------------------------------------------------------------------------------------------------------------------------
Rgba Game::GetColorForGeometry(int type, bool isSleeping) const
{
	Rgba color;

	switch (type)
	{
	case PxGeometryType::eBOX:
	{
		if (isSleeping)
		{
			color = Rgba::DARK_GREY;
		}
		else
		{
			color = Rgba(0.f, 0.4f, 0.f, 1.f);
		}
	}
	break;
	case PxGeometryType::eSPHERE:
	{
		if (isSleeping)
		{
			color = Rgba::DARK_GREY;
		}
		else
		{
			color = Rgba::WHITE;
		}
	}
	break;
	case PxGeometryType::eCONVEXMESH:
	{
		if (isSleeping)
		{
			color = Rgba::DARK_GREY;
		}
		else
		{
			color = Rgba::ORGANIC_BLUE;
		}
	}
	break;
	case PxGeometryType::eCAPSULE:
	{
		if (isSleeping)
		{
			color = Rgba::DARK_GREY;
		}
		else
		{
			color = Rgba::RED;
		}
	}
	default:
		break;
	}

	return color;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::AddMeshForPxCube(CPUMesh& boxMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const
{
	PxBoxGeometry box;
	shape.getBoxGeometry(box);
	Vec3 halfExtents = g_PxPhysXSystem->PxVectorToVec(box.halfExtents);
	PxMat44 pxTransform = actor.getGlobalPose();
	PxVec3 pxPosition = pxTransform.getPosition();

	Matrix44 pose;
	pose.SetIBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column0));
	pose.SetJBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column1));
	pose.SetKBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column2));
	pose.SetTBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column3));

	AABB3 boxShape = AABB3(-1.f * halfExtents, halfExtents);
	boxShape.TransfromUsingMatrix(pose);

	CPUMeshAddCube(&boxMesh, boxShape, color);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::AddMeshForPxSphere(CPUMesh& sphereMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const
{
	PxSphereGeometry sphere;
	shape.getSphereGeometry(sphere);

	PxMat44 pxTransform = actor.getGlobalPose();
	PxVec3 pxPosition = pxTransform.getPosition();

	float radius = sphere.radius;

	Matrix44 pose;
	pose.SetIBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column0));
	pose.SetJBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column1));
	pose.SetKBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column2));
	pose.SetTBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column3));

	CPUMeshAddUVSphere(&sphereMesh, Vec3::ZERO, radius, color, 16, 8);

	int numVerts = sphereMesh.GetVertexCount();
	int limit = numVerts - ((16 + 1) * (8 + 1));

	sphereMesh.TransformVerticesInRange(limit, numVerts, pose);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::AddMeshForPxCapsule(CPUMesh& capMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const
{
	PxCapsuleGeometry capsule;
	shape.getCapsuleGeometry(capsule);

	PxMat44 pxTransform = actor.getGlobalPose();
	PxVec3 pxPosition = pxTransform.getPosition();

	float radius = capsule.radius;

	Matrix44 pose;
	pose.SetIBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column0));
	pose.SetJBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column1));
	pose.SetKBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column2));
	pose.SetTBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column3));

	float halfHeight = capsule.halfHeight;
	Vec3 heightOffset = Vec3(0.f, halfHeight, 0.f);
	Vec3 start = Vec3::ZERO + heightOffset;
	Vec3 end = Vec3::ZERO - heightOffset;

	CPUMeshAddUVCapsule(&capMesh, start, end, radius, color, 16, 8);

	int numVerts = capMesh.GetVertexCount();
	int limit = numVerts - ((16 + 1) * (8 + 1));

	Matrix44 rotationMatrix = Matrix44::MakeZRotationDegrees(90.f);

	capMesh.TransformVerticesInRange(limit, numVerts, rotationMatrix);
	capMesh.TransformVerticesInRange(limit, numVerts, pose);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::AddMeshForConvexMesh(CPUMesh& cvxMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const
{
	PxConvexMeshGeometry convexGeometry;
	shape.getConvexMeshGeometry(convexGeometry);

	PxConvexMesh* pxCvxMesh = convexGeometry.convexMesh;
	const int nbPolys = pxCvxMesh->getNbPolygons();
	const uint8_t* polygons = pxCvxMesh->getIndexBuffer();
	const PxVec3* verts = pxCvxMesh->getVertices();
	int nbVerts = pxCvxMesh->getNbVertices();
	PX_UNUSED(nbVerts);

	PxMat44 pxTransform = actor.getGlobalPose() * shape.getLocalPose();
	PxVec3 pxPosition = pxTransform.getPosition();

	Matrix44 pose;
	pose.SetIBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column0));
	pose.SetJBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column1));
	pose.SetKBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column2));
	pose.SetTBasis(g_PxPhysXSystem->PxVectorToVec(pxTransform.column3));

	int numTotalTriangles = 0;
	for (int index = 0; index < nbPolys; index++)
	{
		PxHullPolygon data;
		pxCvxMesh->getPolygonData(index, data);

		const int nbTris = int(data.mNbVerts - 2);
		const int vref0 = polygons[data.mIndexBase + 0];
		PX_ASSERT(vref0 < nbVerts);
		for (int jIndex = 0; jIndex < nbTris; jIndex++)
		{
			const int vref1 = polygons[data.mIndexBase + 0 + jIndex + 1];
			const int vref2 = polygons[data.mIndexBase + 0 + jIndex + 2];

			//generate face normal:
			PxVec3 e0 = verts[vref1] - verts[vref0];
			PxVec3 e1 = verts[vref2] - verts[vref0];

			PX_ASSERT(vref1 < nbVerts);
			PX_ASSERT(vref2 < nbVerts);

			PxVec3 fnormal = e0.cross(e1);
			fnormal.normalize();

			VertexMaster vert;
			vert.m_color = color;
			if (numTotalTriangles * 3 < 1024)
			{
				Vec3 position = g_PxPhysXSystem->PxVectorToVec(verts[vref0]);
				position = pose.TransformPosition3D(position);

				vert.m_position = position;
				vert.m_normal = g_PxPhysXSystem->PxVectorToVec(fnormal);
				cvxMesh.AddVertex(vert);

				position = g_PxPhysXSystem->PxVectorToVec(verts[vref2]);
				position = pose.TransformPosition3D(position);
				vert.m_position = position;
				cvxMesh.AddVertex(vert);

				position = g_PxPhysXSystem->PxVectorToVec(verts[vref1]);
				position = pose.TransformPosition3D(position);
				vert.m_position = position;
				cvxMesh.AddVertex(vert);

				numTotalTriangles++;
			}
		}
	}

	int vertCount = cvxMesh.GetVertexCount();
	for (int indexIndex = 0; indexIndex < vertCount; indexIndex++)
	{
		cvxMesh.AddIndex(indexIndex);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::SetFrameColorTargetOnCameras() const
{
	//Get the ColorTargetView from rendercontext
	ColorTargetView *colorTargetView = g_renderContext->GetFrameColorTarget();

	//Setup what we are rendering to
	m_mainCamera->SetColorTarget(colorTargetView);
	m_mainCamera->SetViewport(Vec2(0.5f, 0.f), Vec2::ONE);
	m_devConsoleCamera->SetColorTarget(colorTargetView);

	m_UICamera->SetColorTarget(colorTargetView);
	m_UICamera->SetViewport(Vec2::ZERO, Vec2::ONE);

	if (m_initiateFromMenu)
	{
		//The cars exist and we can set the split screen system's color targets
		m_splitScreenSystem.SetColorTargets(colorTargetView);
		m_splitScreenSystem.ComputeViewPortSplits(eSplitMode::PREFER_VERTICAL_SPLIT);
		SetCarHUDColorTargets(colorTargetView);
		SetupCarHUDsFromSplits(eSplitMode::PREFER_VERTICAL_SPLIT);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::DebugRenderToScreen() const
{
	Camera& debugCamera = g_debugRenderer->Get2DCamera();
	debugCamera.m_colorTargetView = g_renderContext->GetFrameColorTarget();
	
	g_renderContext->BindShader(m_shader);
	g_renderContext->BeginCamera(debugCamera);
	
	g_debugRenderer->DebugRenderToScreen();

	g_renderContext->EndCamera();
	
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::DebugRenderToCamera() const
{
	Camera& debugCamera3D = *m_mainCamera;
	debugCamera3D.m_colorTargetView = g_renderContext->GetFrameColorTarget();

	g_renderContext->BindShader(m_shader);

	g_renderContext->BeginCamera(debugCamera3D);
	
	g_debugRenderer->Setup3DCamera(&debugCamera3D);
	g_debugRenderer->DebugRenderToCamera();

	g_renderContext->EndCamera();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PostRender()
{
	//Debug bools
	m_consoleDebugOnce = true;

	if(!m_isDebugSetup)
	{
		//SetStartupDebugRenderObjects();

		ColorTargetView* ctv = g_renderContext->GetFrameColorTarget();
		//Setup debug render client data
		g_debugRenderer->SetClientDimensions( ctv->m_height, ctv->m_width );

		m_isDebugSetup = true;
	}

	//All screen Debug information
	//DebugRenderToScreen();

	if (m_enableImGUI)
	{
		g_ImGUI->Render();
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::PerformFPSCachingAndCalculation(float deltaTime)
{
	m_fpsCache[m_fpsCacheIndex] = m_fpsLastFrame = 1.f / deltaTime;

	if (m_fpsLastFrame > m_fpsHighest)
	{
		m_fpsHighest = m_fpsLastFrame;
	}

	if (m_fpsLastFrame < m_fpsLowest)
	{
		m_fpsLowest = m_fpsLastFrame;
	}

	m_deltaTime = deltaTime;

	m_fpsCacheIndex++;
	if (m_fpsCacheIndex == 1000)
	{
		m_fpsCacheIndex = 0;
	}

	int entriesCounted = 0;
	m_avgFPS = 0.f;
	for (int fpsIndex = 0; fpsIndex < 1000; fpsIndex++)
	{
		if (m_fpsCache[fpsIndex] == 0.f)
		{
			//This entry has not been filled yet
			continue;
		}

		m_avgFPS += m_fpsCache[fpsIndex];
		entriesCounted++;
	}

	m_avgFPS /= entriesCounted;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::Update(float deltaTime)
{
	if (m_numConnectedPlayers == 0)
		return;	//Currently unsupported for keyboard input

	//Finishing up with the loading threads and work
	//Un-comment when we want to use threaded model loading
// 	if (IsFinishedImageLoading() && !m_threadedLoadComplete)	//&& IsFinishedModelLoading() 
// 	{
// 		SetMeshesAndJoinThreads();
// 	}
	
	if (!m_threadedLoadComplete)
	{
		//FinishReadyModels();
		FinishReadyImages();
		return;
	}

	if (m_threadedLoadComplete)
	{
		//Once everything is loaded we want to start calculation
		PerformFPSCachingAndCalculation(deltaTime);
	}

	g_renderContext->m_frameCount++;
	//float currentTime = static_cast<float>(GetCurrentTimeSeconds());

	//Update the camera's transform
	Matrix44 camTransform = Matrix44::MakeFromEuler( m_mainCamera->GetEuler(), m_rotationOrder ); 
	camTransform = Matrix44::SetTranslation3D(m_camPosition, camTransform);
	m_mainCamera->SetModelMatrix(camTransform);

	m_racetrackTransform = Matrix44::SetTranslation3D(m_racetrackTranslation, m_racetrackTransform);
	m_trackTestTransform = Matrix44::SetTranslation3D(m_trackTestTranslation, m_trackTestTransform);
	
	if (m_enableImGUI)
	{
		UpdateImGUI();
	}

	UpdateAllCars(deltaTime);
	CheckForRaceCompletion();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::FixedUpdate(float deltaTime)
{
	UpdateCarCamera(deltaTime);
	UpdatePhysXCar(deltaTime);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdatePhysXCar(float deltaTime)
{
	if (!m_threadedLoadComplete)
	{
		return;
	}

	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		m_cars[carIndex]->FixedUpdate(deltaTime);
	}

}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateCarCamera(float deltaTime)
{
	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		m_cars[carIndex]->UpdateCarCamera(deltaTime);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateImGUI()
{
	UpdateImGUIPhysXWidget();
	UpdateImGUIDebugWidget();

	UpdateImGUIVehicleTool();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateImGUIPhysXWidget()
{
	//This is going to be our main PhysX Scene widget to modify car values and other stuff we may want to change

	//Read Cam Position
	ui_camPosition[0] = m_camPosition.x;
	ui_camPosition[1] = m_camPosition.y;
	ui_camPosition[2] = m_camPosition.z;

	Vec3 cameraAngle = m_mainCamera->GetEuler();
	float cameraAngleFloat[3];
	cameraAngleFloat[0] = cameraAngle.x;
	cameraAngleFloat[1] = cameraAngle.y;
	cameraAngleFloat[2] = cameraAngle.z;

	float vehicleHeightOffset = m_offsetCarBody.y;

	//Create the actual ImGUI widget
	ImGui::Begin("PhysX Scene Controls");                          

	ImGui::DragFloat3("Main Camera Position", ui_camPosition);
	ImGui::DragFloat3("Main Camera Angle", cameraAngleFloat);

	ImGui::DragFloat("Car body height offset", &vehicleHeightOffset);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	//Write CamPos
	m_camPosition.x = ui_camPosition[0];
	m_camPosition.y = ui_camPosition[1];
	m_camPosition.z = ui_camPosition[2];

	m_directionalLightPos.x = ui_dirLight[0];
	m_directionalLightPos.y = ui_dirLight[1];
	m_directionalLightPos.z = ui_dirLight[2];

	m_directionalLightPos.Normalize();

	m_offsetCarBody.y = vehicleHeightOffset;

	ImGui::End();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateImGUIDebugWidget()
{
	//Create a debug widget to enable and disbale certain things when it comes to rendering
	//Properties in the scene or the car itself
	
	//Directional Light UI values
	ui_dirLight[0] = m_directionalLightPos.x;
	ui_dirLight[1] = m_directionalLightPos.y;
	ui_dirLight[2] = m_directionalLightPos.z;
	
	//Track piece translation 
	ui_racetrackTranslation[0] = m_racetrackTranslation.x;
	ui_racetrackTranslation[1] = m_racetrackTranslation.y;
	ui_racetrackTranslation[2] = m_racetrackTranslation.z;

	ImGui::Begin("PhysX Scene Debug Widget");

	ImGui::ColorEdit3("Scene Background Color", (float*)&ui_cameraClearColor); // Edit 3 floats representing a color
	ImGui::DragFloat3("Light Direction", ui_dirLight);
	ImGui::Checkbox("Enable Debug Camera", &ui_swapToMainCamera);
	ImGui::DragFloat3("Track Translation", ui_racetrackTranslation);

	float position[3];
	Vec3 carPosition = m_cars[0]->GetCarController().GetVehiclePosition();
	position[0] = carPosition.x;
	position[1] = carPosition.y;
	position[2] = carPosition.z;

	ImGui::DragFloat3("Car Position", (float*)&position);

	ImGui::Checkbox("Enable Convex Hull Debug", &ui_enableConvexHullRenders);
	ImGui::Checkbox("Enable Car Debug", &ui_enableCarDebug);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	m_directionalLightPos.x = ui_dirLight[0];
	m_directionalLightPos.y = ui_dirLight[1];
	m_directionalLightPos.z = ui_dirLight[2];
	m_directionalLightPos.Normalize();

	m_racetrackTranslation.x = ui_racetrackTranslation[0];
	m_racetrackTranslation.y = ui_racetrackTranslation[1];
	m_racetrackTranslation.z = ui_racetrackTranslation[2];

	ImGui::End();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateImGUIVehicleTool()
{
	m_carTool.UpdateImGUICarTool();

	ImGui::Begin("Reset Cars using Tool data");

	bool result = ImGui::Button("Click To Reset Cars");
	if (result)
	{
		//Create new cars here
		DebuggerPrintf("\n Creating new cars");
		ResetCarsUsingToolData();
	}
	

	ImGui::End();
}

//------------------------------------------------------------------------------------------------------------------------------
bool Game::IsAlive()
{
	//Check if alive
	return m_isGameAlive;
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::RenderGearNumber(int carIndex) const
{
	//Need to draw the gear indicator here
	Camera& HUD = m_cars[carIndex]->GetCarHUDCamera();
	HUD.SetColorTarget(g_renderContext->GetFrameColorTarget());

	g_renderContext->BeginCamera(HUD);
	g_renderContext->BindTextureViewWithSampler(0U, m_menuFont->GetTexture());
	g_renderContext->BindShader(m_shader);
	std::vector<Vertex_PCU> verts;
	std::string textValue = std::to_string(m_cars[carIndex]->GetCarController().GetVehicle()->mDriveDynData.getCurrentGear());
	m_menuFont->AddVertsForText2D(verts, Vec2((HUD.GetOrthoTopRight().x * 0.5f) + 50.f * carIndex, 100.f), 10.f, textValue.c_str(), Rgba::RED);
	g_renderContext->DrawVertexArray(verts);

	g_renderContext->EndCamera();
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreateWayPoints()
{
	for (int carIndex = 0; carIndex < m_numConnectedPlayers; carIndex++)
	{
		WaypointSystem& waypoints = m_cars[carIndex]->GetWaypointsEditable(); 
		waypoints.Startup();

		for (int waypointIndex = 0; waypointIndex < 5; waypointIndex++)
		{
			waypoints.AddNewWayPoint(m_wayPointPositions[waypointIndex], m_wayPointHalfExtents[waypointIndex], waypointIndex);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadGameMaterials()
{
	m_couchMaterial = g_renderContext->CreateOrGetMaterialFromFile(m_couchMaterialPath);
	m_defaultMaterial = g_renderContext->CreateOrGetMaterialFromFile(m_defaultMaterialPath);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::UpdateLightPositions()
{
	g_renderContext->EnableDirectionalLight(Vec3(1.f, 10.f, 1.f), Vec3(0.f, -1.f, -1.f).GetNormalized());
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreateInitialLight()
{
	g_renderContext->EnableDirectionalLight(Vec3(1.f, 10.f, 1.f), Vec3(0.f, -1.f, -1.f).GetNormalized());
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::CreateInitialMeshes()
{
 	CPUMesh mesh;
	mesh.Clear();
	CPUMeshAddQuad(&mesh, AABB2(Vec2(-1000.f, -1000.f), Vec2(1000.f, 1000.f)), Rgba::ORGANIC_PURPLE);

	//mesh.SetLayout<Vertex_Lit>();
	m_baseQuad = new GPUMesh( g_renderContext ); 
	m_baseQuad->CreateFromCPUMesh<Vertex_Lit>( &mesh, GPU_MEMORY_USAGE_STATIC );

	m_baseQuadTransform = Matrix44::IDENTITY;
	m_baseQuadTransform = Matrix44::MakeFromEuler(Vec3(-90.f, 0.f, 0.f));
	m_baseQuadTransform = Matrix44::SetTranslation3D(Vec3(0.f, 0.f, 0.f), m_baseQuadTransform);

	m_pxCube = new GPUMesh(g_renderContext);
	m_pxSphere = new GPUMesh(g_renderContext);
	m_pxConvexMesh = new GPUMesh(g_renderContext);
	m_pxCapMesh = new GPUMesh(g_renderContext);
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadGameTextures()
{
	m_textureTest = g_renderContext->CreateOrGetTextureViewFromFile(m_testImagePath);
	m_boxTexture = g_renderContext->CreateOrGetTextureViewFromFile(m_boxTexturePath);
	m_sphereTexture = g_renderContext->CreateOrGetTextureViewFromFile(m_sphereTexturePath);
	m_floorTexture = g_renderContext->CreateOrGetTextureViewFromFile("ORGANIC_GREEN.png");
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::LoadGameTexturesThreaded()
{
	StartLoadingImage(m_testImagePath);
	StartLoadingImage(m_boxTexturePath);
	StartLoadingImage(m_sphereTexturePath);

	int coreCount = std::thread::hardware_concurrency();
	int halfCores = coreCount / 2;
	for (int i = 0; i < halfCores; ++i)
	{
		m_threads.emplace_back(&Game::ImageLoadThread, this);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void Game::GetandSetShaders()
{
	//Get the Shader
	m_shader = g_renderContext->CreateOrGetShaderFromFile(m_unlitShaderPath);
	m_shader->SetDepth(eCompareOp::COMPARE_LEQUAL, true);

	m_normalShader = g_renderContext->CreateOrGetShaderFromFile(m_normalColorShader);
	m_normalShader->SetDepth(eCompareOp::COMPARE_LEQUAL, true);

	m_defaultLit = g_renderContext->CreateOrGetShaderFromFile(m_litShaderPath);
	m_defaultLit->SetDepth(eCompareOp::COMPARE_LEQUAL, true);
}

/*
//------------------------------------------------------------------------------------------------------------------------------
void Game::HandleMouseInputs(float deltaTime)
{
	//Get pitch and yaw from mouse
	IntVec2 mouseRelativePos = g_windowContext->GetClientMouseRelativeMovement();
	Vec2 mouse = Vec2((float)mouseRelativePos.x, (float)mouseRelativePos.y);

	// we usually want to scale the pixels so we can think of it
	// as a rotational velocity;  Work with these numbers until 
	// it feels good to you; 
	Vec2 scalingFactor = Vec2( 10.f, 10.f ); 
	Vec2 turnSpeed = mouse * scalingFactor; 

	// y mouse movement would corresond to rotation around right (x for us)
	// and x mouse movement corresponds with movement around up (y for us)
	Vec3 camEuler = m_mainCamera->GetEuler();
	camEuler -= deltaTime * Vec3( turnSpeed.y, turnSpeed.x, 0.0f ); 
	m_mainCamera->SetEuler(camEuler);

	// Let's fix our "pitch", or rotation around right to be limited to -85 to 85 degrees (no going upside down)
	camEuler.x = Clamp( camEuler.x, -85.0f, 85.0f );

	// Next, let's keep the turning as between 0 and 360 (some people prefer -180.0f to 180.0f)
	// either way, we just want to keep it a single revolution
	// Note: modf does not correctly mod negative numbers (it'll ignore the sign and mod them as if 
	// they were positive), so I write a special mod function to take this into account;  
	//m_camEuler.y = Modf( m_camEuler.y, 360.0f ); 

	// Awesome, I now have my euler, let's construct a matrix for it;
	// this gives us our current camera's orientation.  we will 
	// use this to translate our local movement to a world movement 
	Matrix44 camMatrix = Matrix44::MakeFromEuler( camEuler ); 

	//Test implementation
	//m_camEuler.y -= static_cast<float>(mouseRelativePos.x);
	//m_camEuler.x -= static_cast<float>(mouseRelativePos.y);
}
*/