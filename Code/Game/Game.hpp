//------------------------------------------------------------------------------------------------------------------------------
#pragma once
//Engine Systems
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/Vertex_PCU.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/IsoSpriteDefenition.hpp"
//Game Systems
#include "Game/CarCamera.hpp"
#include "Game/CarController.hpp"
#include "Game/GameCommon.hpp"
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

struct Camera;

//------------------------------------------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();
	
	static bool TestEvent(EventArgs& args);
	static bool ToggleLight1(EventArgs& args);
	static bool ToggleLight2(EventArgs& args);
	static bool ToggleLight3(EventArgs& args);
	static bool ToggleLight4(EventArgs& args);
	static bool ToggleAllPointLights(EventArgs& args);

	void								StartUp();
	
	void								SetupMouseData();
	void								SetupCameras();
	void								GetandSetShaders();
	void								LoadGameTextures();
	void								CreateIsoSpriteDefenitions();
	void								LoadGameMaterials();
	void								CreateInitialMeshes();
	void								CreateInitialLight();
	void								SetStartupDebugRenderObjects();
	void								SetupPhysX();

	void								CreatePhysXVehicleBoxWall();
	void								CreateObstacleWall(const int numHorizontalBoxes, const int numVerticalBoxes, const float boxSize, const PxVec3& pos, const PxQuat& quat);
	void								CreatePhysXVehicleRamp();
	void								CreatePhysXVehicleObstacles();
	void								CreatePhysXArticulationChain();
	void								CreatePhysXChains(const Vec3& position, int length, const PxGeometry& geometry, float separation);
	void								CreatePhysXConvexHull();
	void								CreatePhysXStack(const Vec3& position, uint size, float halfExtent);

	void								HandleKeyPressed(unsigned char keyCode);
	void								HandleKeyReleased( unsigned char keyCode );
	void								HandleCharacter( unsigned char charCode );

	bool								HandleMouseScroll(float wheelDelta);

	void								DebugEnabled();
	void								Shutdown();

	void								Render() const;
	void								RenderUsingMaterial() const;
	
	void								RenderPhysXScene() const;
	void								RenderPhysXCar() const;
	void								RenderPhysXActors(const std::vector<PxRigidActor*> actors, int numActors, Rgba& color) const;
	
	//For now let's say this is no longer necessary 
	//void								RenderPhysXShapesForVehicle(const std::vector<PxShape*> shapes, int numShapes, Rgba& color) const;
	
	Rgba								GetColorForGeometry(int type, bool isSleeping) const;
	void								AddMeshForPxCube(CPUMesh& boxMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const;
	void								AddMeshForPxSphere(CPUMesh& sphereMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const;
	void								AddMeshForPxCapsule(CPUMesh& capMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const;
	void								AddMeshForConvexMesh(CPUMesh& cvxMesh, const PxRigidActor& actor, const PxShape& shape, const Rgba& color) const;
	
	//void								RenderIsoSprite() const;
	void								DebugRenderToScreen() const;
	void								DebugRenderToCamera() const;
	
	void								PostRender();
	
	void								Update( float deltaTime );
	void								UpdatePhysXCar( float deltaTime );
	void								UpdateCarCamera(float deltaTime);
	void								UpdateImGUI();
	void								UpdateImGUIPhysXWidget();
	void								UpdateImGUIDebugWidget();
	void								UpdateMouseInputs(float deltaTime);
	void								UpdateLightPositions();
	
	bool								IsAlive();
private:
	bool								m_isGameAlive = false;
	bool								m_consoleDebugOnce = false;
	bool								m_devConsoleSetup = false;
	bool								m_isDebugSetup = false;
	float								m_cameraSpeed = 0.3f; 

	CarController*						m_carController = nullptr;

public:
	
	TextureView*						m_textureTest = nullptr;
	TextureView*						m_boxTexture = nullptr;
	TextureView*						m_sphereTexture = nullptr;
	BitmapFont*							m_squirrelFont = nullptr;
	Image*								m_testImage = nullptr;
	float								m_animTime = 0.f;

	//D3D11 stuff
	Shader*								m_shader = nullptr;
	Shader*								m_normalShader = nullptr;
	Shader*								m_defaultLit = nullptr;
	std::string							m_defaultShaderPath = "default_unlit.00.hlsl";
	std::string							m_shaderLitPath = "default_lit.hlsl";
	std::string							m_normalColorShader = "normal_shader.hlsl";
	std::string							m_testImagePath = "Test_StbiFlippedAndOpenGL.png";
	std::string							m_boxTexturePath = "woodcrate.jpg";
	std::string							m_sphereTexturePath = "2k_earth_daymap.jpg";
	std::string							m_xmlShaderPath = "default_unlit.xml";
	std::string							m_couchMaterialPath = "couch.mat";
	std::string							m_defaultMaterialPath = "default.mat";
	std::string							m_carMeshPath = "Car/Car.mesh";	
	std::string							m_wheelMeshPath = "Car/Wheel.mesh";
	std::string							m_wheelFlippedMeshPath = "Car/WheelFlipped.mesh";

	Camera*								m_mainCamera = nullptr;
	Camera*								m_devConsoleCamera = nullptr;
	Rgba*								m_clearScreenColor = nullptr;
	
	float								m_camFOVDegrees = 60.f; //Desired Camera Field of View
	eRotationOrder						m_rotationOrder = ROTATION_ORDER_DEFAULT;
	Vec3								m_camPosition = Vec3::ZERO;

	float								m_devConsoleScreenWidth = 0.f;
	float								m_devConsoleScreenHeight = 0.f;

	// Define the shapes, and how are they positionedin the world; 
	GPUMesh*							m_cube = nullptr; 
	Matrix44							m_cubeTransform; // cube's model matrix

	GPUMesh*							m_sphere = nullptr;
	Matrix44							m_sphereTransform;   // sphere's model matrix

	GPUMesh*							m_quad = nullptr;
	Matrix44							m_quadTransfrom;

	GPUMesh*							m_baseQuad = nullptr;
	Matrix44							m_baseQuadTransform;

	GPUMesh*							m_capsule = nullptr;
	Matrix44							m_capsuleModel;

	GPUMesh*							m_carModel = nullptr;
	Vec4								m_offsetCarBody = Vec4(0.f, -0.5f, 0.f, 0.f);
	GPUMesh*							m_wheelModel = nullptr;
	GPUMesh*							m_wheelFlippedModel = nullptr;
	TextureView*						m_carDiffuse = nullptr;
	TextureView*						m_carNormal = nullptr;

	//Lighting data
	int									m_lightSlot;
	float								m_ambientIntensity = 1.f;
	float								m_ambientStep = 0.1f;

	bool								m_enableDirectional = false;
	bool								m_normalMode = false;

	Vec3								m_directionalLightPos;

	//Material
	Material*							m_couchMaterial = nullptr;
	Material*							m_defaultMaterial = nullptr;
	bool								m_useMaterial = true;

	float								m_emissiveFactor = 0.f;
	float								m_emissiveStep = 0.1f;

	//------------------------------------------------------------------------------------------------------------------------------
	// User Interface Variables
	//------------------------------------------------------------------------------------------------------------------------------

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

	//------------------------------------------------------------------------------------------------------------------------------
	// PhysX Test Variables
	//------------------------------------------------------------------------------------------------------------------------------

	//PhysX Meshes
	GPUMesh*							m_pxCube = nullptr;
	GPUMesh*							m_pxSphere = nullptr;
	GPUMesh*							m_pxConvexMesh = nullptr;
	GPUMesh*							m_pxCapMesh = nullptr;

	bool								m_debugViewCarCollider = false;

	//------------------------------------------------------------------------------------------------------------------------------
	//Car Camera and other game data
	//------------------------------------------------------------------------------------------------------------------------------
	CarCamera*							m_carCamera = nullptr;
	float								m_frameZoomDelta = 0.f;
};