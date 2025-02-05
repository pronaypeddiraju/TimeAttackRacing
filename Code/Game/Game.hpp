//------------------------------------------------------------------------------------------------------------------------------
#pragma once
//Engine Systems
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vertex_PCU.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/IsoSpriteDefenition.hpp"
#include "Engine/Core/Async/AsyncQueue.hpp"
//Game Systems
#include "Game/CarCamera.hpp"
#include "Game/CarController.hpp"
#include "Game/GameCommon.hpp"
#include "Game/WaypointTriggerBased.hpp"
#include "Game/WaypointRegionBased.hpp"
#include "Game/WaypointSystem.hpp"
#include "Game/SplitScreenSystem.hpp"
#include "Game/Car.hpp"
#include "Game/GameplayWork.hpp"
#include "Game/CarTool.hpp"
//Third Party
#include "extensions/PxDefaultAllocator.h"
#include "extensions/PxDefaultCpuDispatcher.h"
#include "extensions/PxDefaultErrorCallback.h"
#include "PxFoundation.h"
#include "pvd/PxPvd.h"
#include "PxRigidDynamic.h"

#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}

using namespace physx; 

//------------------------------------------------------------------------------------------------------------------------------
class Texture;
class BitmapFont;
class TextureView;
class Image;
class Shader;
class SpriteAnimDefenition;
class CPUMesh;
class GPUMesh;
class Model;
class UIWidget;
class UIRadioGroup;
class UIButton;

struct Camera;

//------------------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();
		
	void								StartUp();
	void								DebugEnabled();
	void								Shutdown();
	void								Update(float deltaTime);
	void								FixedUpdate(float deltaTime);
	void								UpdateImGUI();

	//For Audio setup reusing existing audio IDs
	void								CopyAudioIDsFromFirstCar(int carIndex);

	//Only for debug rendering available debug shapes in the scene
	void								SetStartupDebugRenderObjects();

	//Input Handling
	void								HandleKeyPressed(unsigned char keyCode);
	void								HandleKeyReleased(unsigned char keyCode);
	void								HandleCharacter(unsigned char charCode);

	bool								HandleMouseScroll(float wheelDelta);
	//void								HandleMouseInputs(float deltaTime);

	//Create PhysX World Objects
	void								CreatePhysXVehicleBoxWall();
	void								CreateObstacleWall(const int numHorizontalBoxes, const int numVerticalBoxes, const float boxSize, const PxVec3& pos, const PxQuat& quat);
	void								CreatePhysXVehicleRamp();
	void								CreatePhysXVehicleObstacles();

	void								CreatePhysXConvexHull();
	void								CreatePhysXStack(const Vec3& position, uint size, float halfExtent);
	
	//Vehicle Reset
	void								ResetCarPositionForPlayer(int playerID);

	//Race Logic and Systems
	void								CreateWayPoints();
	PxRigidActor*						GetCarActor() const;

	//Render Logic
	void								Render() const;
	
	//For now let's say this is no longer necessary 
	//void								RenderPhysXShapesForVehicle(const std::vector<PxShape*> shapes, int numShapes, Rgba& color) const
	
	void								PostRender();
	
	void								PerformFPSCachingAndCalculation(float deltaTime);
	void								CheckForGameStart();
	void								UpdatePhysXCar(float deltaTime);
	void								UpdateCarCamera(float deltaTime);
	
	void								UpdateImGUIPhysXWidget();
	void								UpdateImGUIDebugWidget();
	void								UpdateImGUIVehicleTool();
	
	void								UpdateLightPositions();
	
	bool								IsAlive();

private:

	//Initial Setups
	void								InitiateGameSequence();
	void								SetupCars();
	void								SetupMouseData();
	void								SetupCameras();
	void								GetandSetShaders();
	void								LoadGameTextures();
	void								LoadGameTexturesThreaded();
	void								LoadGameMaterials();
	void								CreateInitialMeshes();
	void								CreateInitialLight();
	void								SetupPhysX();
	void								CreateUIWidgets();
	void								LoadAudio();

	void								PerformSingleThreadLoading();
	void								LoadTrackMeshesOnSceneCreation();

	void								CreateBaseBoxForCollisionDetection();
	void								ResetCarsUsingToolData();
	void								ReadBestTimeFromFile();
	void								ReadBestTimeFromTextFile();

	//Async Functionality 
	void								PerformAsyncLoading();
	void								EnqueueLoadingModel(std::string fileName);
	void								LoadModelsFromThread();
	void								FinishReadyModels();
	bool								IsFinishedModelLoading() const;
	void								SetMeshesAndJoinThreads();

	void								ImageLoadThread();
	void								StartLoadingImage(std::string fileName);
	void								FinishReadyImages();
	bool								IsFinishedImageLoading() const;


	//Update Functions
	void								UpdateAllCars(float deltaTime);
	void								CheckForRaceCompletion();

	void								HandleRaceCompletedCondition();
	void								SetEnableXInput(bool isEnabled = true);

	//Drawing Utilities for PhysX Shapes
	Rgba								GetColorForGeometry(int type, bool isSleeping) const;
	void								AddMeshForPxCube(CPUMesh& boxMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const;
	void								AddMeshForPxSphere(CPUMesh& sphereMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const;
	void								AddMeshForPxCapsule(CPUMesh& capMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const;
	void								AddMeshForConvexMesh(CPUMesh& cvxMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const;

	//Render Functions
	void								SetFrameColorTargetOnCameras() const;
	void								SetAmbientIntensity() const;
	void								SetEmissiveIntensity() const;

	void								DebugRenderToScreen() const;
	void								DebugRenderToCamera() const;

	void								RenderRacetrack() const;
	void								RenderUsingMaterial() const;

	void								RenderSceneForCarCameras() const;
	void								RenderScreenForMainCamera() const;

	void								RenderPhysXScene() const;
	void								RenderPhysXCar(const CarController& carController) const;
	void								RenderPhysXActors(const std::vector<PxRigidActor*> actors, int numActors, Rgba& color) const;

	void								RenderViewportBorders() const;

	//Rendering for Menus
	void								SetCarHUDColorTargets(ColorTargetView * colorTargetView) const;
	void								RenderMainMenu() const;	//Uses UIWidget and widget system
	void								RenderMenuScreen() const;
	void								RenderRaceCompleted() const;

	void								AddVertsForPlayerTimesInOrder(std::vector<Vertex_PCU>& timeVerts) const;
	void								RenderUITest() const;
	void								RenderDebugInfoOnScreen() const;

	void								RenderGearNumber(int carIndex) const;

	void								SetupCarHUDsFromSplits(eSplitMode splitMode) const;

	//Shutdown utils
	void								WriteNewBestTime();
	void								WriteNewBestTimeText();
	void								DeleteUI();

	//Restart level functionality
	void								RestartLevel();
	void								CheckXInputForRestart();

private:
	bool								m_isGameAlive = false;
	bool								m_consoleDebugOnce = false;
	bool								m_devConsoleSetup = false;
	bool								m_isDebugSetup = false;
	bool								m_isXInputEnabled = false;
	bool								m_isRaceCompleted = false;
	float								m_cameraSpeed = 0.3f; 

	float								m_deltaTime = 0.f;
	float								m_fpsCache[1000] = {0.f};
	float								m_fpsLastFrame = 0.f;
	float								m_avgFPS = 0.f;
	int									m_fpsCacheIndex = 0;
	int									m_numConnectedPlayers = 0;

	float								m_fpsLowest = 10.f;
	float								m_fpsHighest = 0.f;

	Car*								m_cars[4] = {nullptr, nullptr, nullptr, nullptr};
	Vec3								m_startPositions[4] = { 
											Vec3(18.f, 10.f, 0.f),
											Vec3(26.f, 10.f, 0.f), 
											Vec3(18.f, 10.f, -10.f), 
											Vec3(26.f, 10.f, -10.f) };

public:
	
	TextureView*						m_textureTest = nullptr;
	TextureView*						m_boxTexture = nullptr;
	TextureView*						m_sphereTexture = nullptr;
	BitmapFont*							m_menuFont = nullptr;
	Image*								m_testImage = nullptr;
	float								m_animTime = 0.f;

	//D3D11 stuff
	Shader*								m_shader = nullptr;
	Shader*								m_normalShader = nullptr;
	Shader*								m_defaultLit = nullptr;
	
	//Async Queues for the loading
	AsyncQueue<ImageLoadWork*>			m_loadQueue;
	AsyncQueue<ImageLoadWork*>			m_finishedQueue;
	int									m_imageLoading = 0;

	AsyncQueue<ModelLoadWork*>			m_modelLoadQueue;
	AsyncQueue<ModelLoadWork*>			m_modelFinishedQueue;
	int									m_modelLoading = 0;

	std::vector<std::thread>			m_threads;
	bool								m_threadedLoadComplete = false;

	//Image Paths
	std::string							m_testImagePath = "Test_StbiFlippedAndOpenGL.png";
	std::string							m_boxTexturePath = "woodcrate.jpg";
	std::string							m_sphereTexturePath = "2k_earth_daymap.jpg";
	
	//Shader Paths
	std::string							m_litShaderPath = "default_lit.hlsl";
	std::string							m_defaultShaderPath = "default_unlit.00.hlsl";
	std::string							m_normalColorShader = "normal_shader.hlsl";
	std::string							m_unlitShaderPath = "default_unlit.xml";
	
	//Material Paths
	std::string							m_couchMaterialPath = "couch.mat";
	std::string							m_defaultMaterialPath = "default.mat";

	//Mesh Paths
	std::string							m_carMeshPath = "Car/Car.mesh";	
	std::string							m_wheelMeshPath = "Car/Wheel.mesh";
	std::string							m_wheelFlippedMeshPath = "Car/WheelFlipped.mesh";
	std::string							m_treeMeshPath = "foliage/pineAllMeshes.mesh";
	std::string							m_trackAngledPath = "Track/angled.mesh";
	std::string							m_trackJumpPath = "Track/jump1.mesh";
	std::string							m_trackTestPath = "ScaledTrack/ScaledTrack1RoadOnly.mesh";
	std::string							m_trackCollisionsTestPath = "ScaledTrack/ScaledTrack1CollidersOnly.mesh";

	Camera*								m_mainCamera = nullptr;
	Camera*								m_devConsoleCamera = nullptr;
	Camera*								m_UICamera = nullptr;

	AABB2								m_UIBounds;
	float								m_fontHeight = 20.0f;
	float								m_menuFontHeight = 50.f;

	Rgba*								m_clearScreenColor = nullptr;
	
	float								m_camFOVDegrees = 60.f; //Desired Camera Field of View
	eRotationOrder						m_rotationOrder = ROTATION_ORDER_DEFAULT;
	Vec3								m_camPosition = Vec3::ZERO;

	float								m_devConsoleScreenWidth = 0.f;
	float								m_devConsoleScreenHeight = 0.f;

	//------------------------------------------------------------------------------------------------------------------------------
	//BGM
	//------------------------------------------------------------------------------------------------------------------------------
	SoundID								m_BGMTrack;
	SoundPlaybackID						m_BGMTrackPlaybackID;
	std::string							m_BGMPath = "Data/Audio/BGMTrack.mp3";

	// Define the shapes, and how are they positionedin the world; 
	GPUMesh*							m_baseQuad = nullptr;
	Matrix44							m_baseQuadTransform;

	//------------------------------------------------------------------------------------------------------------------------------
	// PhysX Meshes and Textures used by car and other PhysX objects
	//------------------------------------------------------------------------------------------------------------------------------

	GPUMesh*							m_carModel = nullptr;
	Vec4								m_offsetCarBody = Vec4(0.f, -0.5f, 0.f, 0.f);
	GPUMesh*							m_wheelModel = nullptr;
	GPUMesh*							m_wheelFlippedModel = nullptr;
	TextureView*						m_carDiffuse = nullptr;
	TextureView*						m_carNormal = nullptr;

	GPUMesh*							m_trackPieceModel = nullptr;
	Vec3								m_racetrackTranslation = Vec3(0.f, 0.f, 0.f);
	Matrix44							m_racetrackTransform;

	GPUMesh*							m_treeModel = nullptr;
	Vec3								m_treeTranslation = Vec3(0.f, 0.f, 0.f);
	Matrix44							m_treeTransform;

	GPUMesh*							m_trackTestModel = nullptr;
	GPUMesh*							m_trackCollidersTestModel = nullptr;
	Vec3								m_trackTestTranslation = Vec3(0.f, -0.3f, 0.f);
	Matrix44							m_trackTestTransform;
	
	//------------------------------------------------------------------------------------------------------------------------------
	//Lighting data
	//------------------------------------------------------------------------------------------------------------------------------
	
	int									m_lightSlot;
	float								m_ambientIntensity = 1.f;
	float								m_ambientStep = 0.1f;

	bool								m_enableDirectional = true;
	bool								m_normalMode = false;

	Vec3								m_directionalLightPos;

	//Material
	Material*							m_couchMaterial = nullptr;
	Material*							m_defaultMaterial = nullptr;
	TextureView*						m_floorTexture = nullptr;
	bool								m_useMaterial = true;

	float								m_emissiveFactor = 0.f;
	float								m_emissiveStep = 0.1f;

	//------------------------------------------------------------------------------------------------------------------------------
	// User Interface Variables
	//------------------------------------------------------------------------------------------------------------------------------

	bool								m_enableImGUI = false;

	float								ui_testSlider = 0.5f;
	float								ui_cameraClearColor[3] = { 0.f, 0.f, 0.f };
	bool								ui_testCheck1 = false;
	bool								ui_testCheck2 = true;
	float								ui_camPosition[3] = { 0.f, 0.f, 0.f };
	float								ui_dirLight[3] = { -1.f, -1.f, -1.f };
	float								ui_dynamicSpawnPos[3] = { -1.f, -1.f, -1.f };
	float								ui_dynamicVelocity[3] = { -1.f, -1.f, -1.f };
	
	float								ui_camTilt = 10.f;
	float								ui_camAngle = 10.f;
	float								ui_camHeight = 10.f;
	float								ui_camDistance = 10.f;
	float								ui_camLerpSpeed = 0.1f;

	bool								ui_enableCarDebug = false;
	bool								ui_swapToMainCamera = false;
	float								ui_racetrackTranslation[3] = { 0.f, 0.f, 0.f };

	bool								ui_enableConvexHullRenders = false;

	//------------------------------------------------------------------------------------------------------------------------------
	// Game Menu variables
	//------------------------------------------------------------------------------------------------------------------------------
	//UI References
	UIWidget*							m_menuParent = nullptr;
	UIRadioGroup*						m_menuRadGroup = nullptr;
	UIButton*							m_playButton = nullptr;
	UIButton*							m_editButton = nullptr;

	bool								m_initiateFromMenu = false;


	//------------------------------------------------------------------------------------------------------------------------------
	// PhysX Test Variables
	//------------------------------------------------------------------------------------------------------------------------------

	//PhysX Meshes
	GPUMesh*							m_pxCube = nullptr;
	GPUMesh*							m_pxSphere = nullptr;
	GPUMesh*							m_pxConvexMesh = nullptr;
	GPUMesh*							m_pxCapMesh = nullptr;

	bool								m_debugViewCarCollider = false;

	Vec3								m_wayPointPositions[5] = { Vec3(22.5f, 0.f, 10.f),
																	Vec3(65.f, 0.f, 126.5f),
																	Vec3(-55.5f, 0.f, 65.f), 
																	Vec3(-37.5f, 0.f, -50.f), 
																	Vec3(67.5f, 0.f, -75.f) };

	Vec3								m_wayPointHalfExtents[5] = { Vec3(6.5f, 5.f, 1.f),
																	Vec3(1.f, 5.f, 6.5f),
																	Vec3(8.5f, 5.f, 1.f),
																	Vec3(6.5f, 5.f, 1.f),
																	Vec3(6.5f, 5.f, 1.f) };

	//------------------------------------------------------------------------------------------------------------------------------
	//Car Camera and other game data
	//------------------------------------------------------------------------------------------------------------------------------
	//CarCamera*							m_carCamera = nullptr;
	//CarCamera*							m_player2CarCamera = nullptr;

	float								m_frameZoomDelta = 0.f;
	
	//------------------------------------------------------------------------------------------------------------------------------
	// Waypoint System
	//------------------------------------------------------------------------------------------------------------------------------
	bool								m_debugRenderWaypoints = false;
	bool								m_debugPerfEnabled = false;
	//Save File data
	double								m_bestTimeFromFile = 0.0;
	double								m_bestTimeForRun = 0.0;
	double								m_previousBestTime = 0.0;
	std::string							m_saveFilePath = "Data/Gameplay/SaveFile.xml";
	std::string							m_saveFileTextPath = "Data/Gameplay/BestTime.txt";
	bool								m_timeBeaten = false;

	//------------------------------------------------------------------------------------------------------------------------------
	// Split Screen System
	//------------------------------------------------------------------------------------------------------------------------------
	SplitScreenSystem					m_splitScreenSystem;
	eSplitMode							m_splitMode = eSplitMode::PREFER_VERTICAL_SPLIT;
	int									m_borderWidth = 20.f;

	//------------------------------------------------------------------------------------------------------------------------------
	// Vehicle Tool
	//------------------------------------------------------------------------------------------------------------------------------
	CarTool								m_carTool;
};