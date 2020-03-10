//  [8/23/2018 Pronay Peddiraju]
#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Commons/EngineCommon.hpp"

class Game;

class App
{
public:
	//Public methods

	App();
	~App();
	
	static bool Command_Quit(EventArgs& args);

	void LoadGameBlackBoard();
	void StartUp();
	void ShutDown();
	void RestartAllSystems();
	void RunFrame();

	bool IsQuitting() const { return m_isQuitting; }
	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );
	bool HandleCharacter( unsigned char charCode);
	bool HandleQuitRequested();

private:
	//Private methods
	void BeginFrame();
	void Update();
	void Render() const;
	void PostRender();
	void EndFrame();

public:
	//public variables

private:
	//private variable
	bool		m_isQuitting = false;
	bool		m_isPaused = false;
	bool		m_isSlowMo = false;
	
	//Minumum frames force wait
	int			m_minFramesToWait = 10;

	Game*		m_game = nullptr;
	
	//Reference to the window handle as void*
	void*		m_appWindowHandle = nullptr;

	double		m_timeAtLastFrameBegin = 0;
	double		m_timeAtThisFrameBegin = 0;

	double		m_timeCacheForFrame = 0;
	double		m_fixedTimeStepForUpdate = 0.01;	//Making the fixed time step really small guarantees at least 1 update per frame
};