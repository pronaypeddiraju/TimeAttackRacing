//------------------------------------------------------------------------------------------------------------------------------
#include "Game/App.hpp"
//Engine Systems
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Commons/Profiler/Profiler.hpp"
#include "Engine/Commons/Profiler/ProfilerReport.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystems.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/XMLUtils/XMLUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/PhysXSystem/PhysXSystem.hpp"
//Game Systems
#include "Game/Game.hpp"

App* g_theApp = nullptr;

App::App()
{	
}

App::~App()
{
	ShutDown();
}

STATIC bool App::Command_Quit(EventArgs& args)
{
	UNUSED(args);
	g_theApp->HandleQuitRequested();
	return true;
}

void App::LoadGameBlackBoard()
{
	const char* xmlDocPath = "Data/Gameplay/GameConfig.xml";
	tinyxml2::XMLDocument gameconfig;
	gameconfig.LoadFile(xmlDocPath);
	
	if(gameconfig.ErrorID() != tinyxml2::XML_SUCCESS)
	{
		printf("\n >> Error loading XML file from %s ", xmlDocPath);
		printf("\n >> Error ID : %i ", gameconfig.ErrorID());
		printf("\n >> Error line number is : %i", gameconfig.ErrorLineNum());
		printf("\n >> Error name : %s", gameconfig.ErrorName());
		ERROR_AND_DIE(">> Error loading GameConfig XML file ")
		return;
	}
	else
	{
		//We read everything fine. Now just shove all that data into the black board
		XMLElement* rootElement = gameconfig.RootElement();
		g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*rootElement);
	}
}

void App::StartUp()
{
	LoadGameBlackBoard();

	g_audio = new AudioSystem();

	g_eventSystem = new EventSystems();

	//This is now being set in Main_Windows.cpp
	//g_renderContext = new RenderContext(m_appWindowHandle);
	
	g_inputSystem = new InputSystem();

	g_devConsole = new DevConsole();
	g_devConsole->Startup();

	//create the networking system
	//g_networkSystem = new NetworkSystem();

	g_debugRenderer = new DebugRender();
	g_debugRenderer->Startup(g_renderContext);

	g_PxPhysXSystem = new PhysXSystem();

	g_ImGUI = new ImGUISystem(g_renderContext);

	g_RNG = new RandomNumberGenerator(0);

#if defined(_DEBUG)
	{
		g_LogSystem = new LogSystem(LOG_PATH);
		g_LogSystem->LogSystemInit();

		gProfiler->ProfilerInitialize();
	}
#endif

	gProfiler->ProfilerInitialize();
	gProfiler->ProfilerSetMaxHistoryTime(3);

	m_game = new Game();
	m_game->StartUp();
	
	g_eventSystem->SubscribeEventCallBackFn("Quit", Command_Quit);
}

void App::ShutDown()
{
	delete g_ImGUI;
	g_ImGUI = nullptr;

	delete g_renderContext;
	g_renderContext = nullptr;

	delete g_inputSystem;
	g_inputSystem = nullptr;

	delete g_audio;
	g_audio = nullptr;

	delete g_PxPhysXSystem;
	g_PxPhysXSystem = nullptr;

	delete g_devConsole;
	g_devConsole = nullptr;

	delete g_eventSystem;
	g_eventSystem = nullptr;

	delete g_debugRenderer;
	g_debugRenderer = nullptr;

	delete g_RNG;
	g_RNG = nullptr;

#if defined(_DEBUG)
	{
		g_LogSystem->LogSystemShutDown();
		delete g_LogSystem;
		g_LogSystem = nullptr;

		gProfiler->ProfilerShutdown();
	}
#endif
	
	gProfiler->ProfilerShutdown();

	m_game->Shutdown();
}

void App::RestartAllSystems()
{
	delete g_ImGUI;
	g_ImGUI = nullptr;

	delete g_audio;
	g_audio = nullptr;

	g_PxPhysXSystem->RestartPhysX();
	
	delete g_debugRenderer;
	g_debugRenderer = nullptr;

	delete g_RNG;
	g_RNG = nullptr;

	m_game->Shutdown();

	g_renderContext->Restart();

	//This is now being set in Main_Windows.cpp
	//g_renderContext = new RenderContext(m_appWindowHandle);

	g_audio = new AudioSystem();

	//create the networking system
	//g_networkSystem = new NetworkSystem();

	g_debugRenderer = new DebugRender();
	g_debugRenderer->Startup(g_renderContext);

	g_ImGUI = new ImGUISystem(g_renderContext);

	g_RNG = new RandomNumberGenerator(0);

	m_game = new Game();
	m_game->StartUp();
}

void App::RunFrame()
{
	BeginFrame();	
	
	Render();	
	Update();

	PostRender();

	EndFrame();
}

void App::BeginFrame()
{
	gProfiler->ProfilerBeginFrame("App::BeginFrame");

	g_renderContext->BeginFrame();
	g_inputSystem->BeginFrame();
	g_audio->BeginFrame();
	g_devConsole->BeginFrame();
	g_eventSystem->BeginFrame();
	g_debugRenderer->BeginFrame();
	g_ImGUI->BeginFrame();
	g_PxPhysXSystem->BeginFrame();
}

void App::EndFrame()
{
	g_renderContext->EndFrame();
	g_inputSystem->EndFrame();
	g_audio->EndFrame();
	g_devConsole->EndFrame();
	g_eventSystem->EndFrame();
	g_debugRenderer->EndFrame();
	g_ImGUI->EndFrame();
	g_PxPhysXSystem->EndFrame();

	gProfiler->ProfilerEndFrame();
}

void App::Update()
{	
	gProfiler->ProfilerUpdate();

	m_timeAtLastFrameBegin = m_timeAtThisFrameBegin;
	m_timeAtThisFrameBegin = GetCurrentTimeSeconds();

	m_timeCacheForFrame += m_timeAtThisFrameBegin - m_timeAtLastFrameBegin;

	if (m_minFramesToWait < 0)
	{
		while (m_timeCacheForFrame > m_fixedTimeStepForUpdate)
		{
			g_devConsole->UpdateConsole((float)m_fixedTimeStepForUpdate);
			g_PxPhysXSystem->Update((float)m_fixedTimeStepForUpdate);
			m_game->FixedUpdate((float)m_fixedTimeStepForUpdate);

			m_timeCacheForFrame -= m_fixedTimeStepForUpdate;
		}
	}
	else
	{
		m_minFramesToWait--;
	}

	float deltaTime = static_cast<float>(m_timeAtThisFrameBegin - m_timeAtLastFrameBegin);

	int timeInMS = (int)(deltaTime * 1000.f);
	if (timeInMS < 16)
	{
		//g_devConsole->PrintString(Rgba::GREEN, Stringf("Delta Time: %d", timeInMS));
	}
	else
	{
		g_devConsole->PrintString(Rgba::RED, Stringf("Delta Time: %d", timeInMS));
	}
	deltaTime = ClampZeroToOne(deltaTime);

	m_game->Update(deltaTime);
}

void App::Render() const
{
	m_game->Render();	
}

void App::PostRender()
{
	m_game->PostRender();
}

bool App::HandleKeyPressed(unsigned char keyCode)
{
	if(keyCode == TILDY_KEY)
	{
		g_devConsole->ToggleOpenFull();
	}

	switch(keyCode)
	{
		case UP_ARROW:
		case RIGHT_ARROW:
		case LEFT_ARROW:	
		case DOWN_ARROW:
		case DEL_KEY:
		case BACK_SPACE:
		case ENTER_KEY:
		case F1_KEY:
		case F2_KEY:
		case F3_KEY:
		case F4_KEY:
		case F5_KEY:
		case F6_KEY:
		case F7_KEY:
		case SPACE_KEY:
		case A_KEY:
		case W_KEY:
		case S_KEY:
		case D_KEY:
		case NUM_1:
		{
			m_game->HandleKeyPressed(keyCode);
			return true;
		}
		case F8_KEY:
		{
			//Kill and restart the app
			RestartAllSystems();

			return true;
		}
		case KEY_ESC:
		{
			if(!g_devConsole->IsOpen())
			{
				//Shut the app
				g_theApp->HandleQuitRequested();
				return true;
			}
			else
			{
				m_game->HandleKeyPressed(keyCode);
				return true;
			}
		}
		default:
			m_game->HandleKeyPressed(keyCode);
			return true;
		break;
	}
}

bool App::HandleKeyReleased(unsigned char keyCode)
{
	switch(keyCode)
	{
		/*
		case 'T':
		{
			//Implement code to return deltaTime to original value
			m_isSlowMo = false;
			return true;
		}
		case  'P':
		{
			//Implement code to un-pause game
			m_isPaused = false;
			return true;
		}
		*/
		case 38:
		case 32:
		case 39:
		case 37:
		{
			m_game->HandleKeyReleased(keyCode);
			return true;
		}
		default:
		return false;
	}
}

bool App::HandleCharacter( unsigned char charCode )
{
	m_game->HandleCharacter(charCode);
	return false;
}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;
	return m_isQuitting;
}