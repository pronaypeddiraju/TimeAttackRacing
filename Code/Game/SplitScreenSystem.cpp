#include "Game/SplitScreenSystem.hpp"
//Engine Systems
#include "Engine/Commons/EngineCommon.hpp"

//------------------------------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////
//Static constant split values
//////////////////////////////////////////////////////////////////////////

//2P Vertical Splits
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_2P_FIRST_MIN = Vec2::ZERO;
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_2P_FIRST_MAX = Vec2(0.5f, 1.f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_2P_SECOND_MIN = Vec2(0.5f, 0.f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_2P_SECOND_MAX = Vec2::ONE;

//2P Horizontal Splits
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_2P_FIRST_MIN = Vec2(0.f, 0.5f);
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_2P_FIRST_MAX = Vec2::ONE;
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_2P_SECOND_MIN = Vec2::ZERO;
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_2P_SECOND_MAX = Vec2(1.f, 0.5f);

//3P Vertical Splits
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_3P_FIRST_MIN = Vec2(0.f, 0.5f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_3P_FIRST_MAX = Vec2(0.5f, 1.f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_3P_SECOND_MIN = Vec2(0.5f, 0.5f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_3P_SECOND_MAX = Vec2::ONE;
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_3P_THIRD_MIN = Vec2::ZERO;
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_3P_THIRD_MAX = Vec2(1.f, 0.5f);

//3P Horizontal Splits
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_3P_FIRST_MIN = Vec2(0.f, 0.5f);
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_3P_FIRST_MAX = Vec2::ONE;
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_3P_SECOND_MIN = Vec2::ZERO;
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_3P_SECOND_MAX = Vec2(0.5f, 0.5f);
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_3P_THIRD_MIN = Vec2(0.5f, 0.f);
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_3P_THIRD_MAX = Vec2(1.f, 0.5f);

//4P Vertical Splits
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_4P_FIRST_MIN = Vec2(0.f, 0.5f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_4P_FIRST_MAX = Vec2(0.5f, 1.f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_4P_SECOND_MIN = Vec2(0.5f, 0.5f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_4P_SECOND_MAX = Vec2::ONE;
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_4P_THIRD_MIN = Vec2::ZERO;
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_4P_THIRD_MAX = Vec2(0.5f, 0.5f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_4P_FOURTH_MIN = Vec2(0.5f, 0.f);
const STATIC Vec2 SplitScreenSystem::VERTICAL_SPLIT_4P_FOURTH_MAX = Vec2(1.f, 0.5f);

//4P Horizontal Splits
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_4P_FIRST_MIN = Vec2(0.f, 0.5f);
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_4P_FIRST_MAX = Vec2(0.f, 1.f);
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_4P_SECOND_MIN = Vec2::ZERO;
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_4P_SECOND_MAX = Vec2(0.5f, 0.5f);
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_4P_THIRD_MIN = Vec2(0.5f, 0.5f);
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_4P_THIRD_MAX = Vec2::ONE;
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_4P_FOURTH_MIN = Vec2(0.5f, 0.f);
const STATIC Vec2 SplitScreenSystem::HORIZONTAL_SPLIT_4P_FOURTH_MAX = Vec2(1.f, 0.5f);

//------------------------------------------------------------------------------------------------------------------------------
SplitScreenSystem::SplitScreenSystem()
{

}

//------------------------------------------------------------------------------------------------------------------------------
SplitScreenSystem::~SplitScreenSystem()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void SplitScreenSystem::AddCarCameraForPlayer(CarCamera* camera, int playerID)
{
	ASSERT_OR_DIE(playerID < 4, "The player ID recieved by SplitScreenSystem is greater than  4");

	if (m_playerCameras[playerID] == nullptr)
	{
		m_playerCameras[playerID] = camera;
		m_numPlayers++;
	}
	else
	{
		ERROR_AND_DIE("PlayerID passed to the SplitScreenSystem was already assigned");
	}

}

//------------------------------------------------------------------------------------------------------------------------------
int SplitScreenSystem::GetNumPlayers() const
{
	return m_numPlayers;
}

//------------------------------------------------------------------------------------------------------------------------------
void SplitScreenSystem::SetNumPlayers(int numPlayers)
{
	m_numPlayers = numPlayers;
}

//------------------------------------------------------------------------------------------------------------------------------
void SplitScreenSystem::ComputeSplits(eSplitMode splitMode) const
{
	ASSERT_OR_DIE(m_numPlayers > 0, "The number of players in the Split Screen System is 0");

	switch (m_numPlayers)
	{
	case 1:
	{
		//We have only 1 player so we should be fine with the viewport being the whole screen
		m_playerCameras[0]->SetViewport(Vec2::ZERO, Vec2::ONE);
	}
	break;
	case 2:
	{
		//We have 2 players, check the split mode for 2P and split accordingly
		if (splitMode == PREFER_VERTICAL_SPLIT)
		{
			//We need to split the screen into vertical halfs
			m_playerCameras[0]->SetViewport(VERTICAL_SPLIT_2P_FIRST_MIN,		VERTICAL_SPLIT_2P_FIRST_MAX);
			m_playerCameras[1]->SetViewport(VERTICAL_SPLIT_2P_SECOND_MIN,	VERTICAL_SPLIT_2P_SECOND_MAX);
		}
		else
		{
			//We need to split the screen into horizontal halfs
			m_playerCameras[0]->SetViewport(HORIZONTAL_SPLIT_2P_FIRST_MIN,	HORIZONTAL_SPLIT_2P_FIRST_MAX);
			m_playerCameras[1]->SetViewport(HORIZONTAL_SPLIT_2P_SECOND_MIN,	HORIZONTAL_SPLIT_2P_SECOND_MAX);
		}
	}
	break;
	case 3:
	{
		if (splitMode == PREFER_VERTICAL_SPLIT)
		{
			m_playerCameras[0]->SetViewport(VERTICAL_SPLIT_3P_FIRST_MIN,		VERTICAL_SPLIT_3P_FIRST_MAX);
			m_playerCameras[1]->SetViewport(VERTICAL_SPLIT_3P_SECOND_MIN,	VERTICAL_SPLIT_3P_SECOND_MAX);
			m_playerCameras[2]->SetViewport(VERTICAL_SPLIT_3P_THIRD_MIN,		VERTICAL_SPLIT_3P_THIRD_MAX);
		}
		else
		{
			m_playerCameras[0]->SetViewport(HORIZONTAL_SPLIT_3P_FIRST_MIN,	HORIZONTAL_SPLIT_3P_FIRST_MAX);
			m_playerCameras[1]->SetViewport(HORIZONTAL_SPLIT_3P_SECOND_MIN,	HORIZONTAL_SPLIT_3P_SECOND_MAX);
			m_playerCameras[2]->SetViewport(HORIZONTAL_SPLIT_3P_THIRD_MIN,	HORIZONTAL_SPLIT_3P_THIRD_MAX);
		}
	}
	break;
	case 4:
	{
		if (splitMode == PREFER_VERTICAL_SPLIT)
		{
			m_playerCameras[0]->SetViewport(VERTICAL_SPLIT_4P_FIRST_MIN,		VERTICAL_SPLIT_4P_FIRST_MAX);
			m_playerCameras[1]->SetViewport(VERTICAL_SPLIT_4P_SECOND_MIN,	VERTICAL_SPLIT_4P_SECOND_MAX);
			m_playerCameras[2]->SetViewport(VERTICAL_SPLIT_4P_THIRD_MIN,		VERTICAL_SPLIT_4P_THIRD_MAX);
			m_playerCameras[3]->SetViewport(VERTICAL_SPLIT_4P_FOURTH_MIN,	VERTICAL_SPLIT_4P_FOURTH_MAX);
		}
		else
		{
			m_playerCameras[0]->SetViewport(HORIZONTAL_SPLIT_4P_FIRST_MIN,	HORIZONTAL_SPLIT_4P_FIRST_MAX);
			m_playerCameras[1]->SetViewport(HORIZONTAL_SPLIT_4P_SECOND_MIN,	HORIZONTAL_SPLIT_4P_SECOND_MAX);
			m_playerCameras[2]->SetViewport(HORIZONTAL_SPLIT_4P_THIRD_MIN,	HORIZONTAL_SPLIT_4P_THIRD_MAX);
			m_playerCameras[3]->SetViewport(HORIZONTAL_SPLIT_4P_FOURTH_MIN,	HORIZONTAL_SPLIT_4P_FOURTH_MAX);
		}
	}
	break;
	default:
		break;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
const std::array<CarCamera*, 4>& SplitScreenSystem::GetAllCameras() const
{
	return m_playerCameras;
}

//------------------------------------------------------------------------------------------------------------------------------
const CarCamera* SplitScreenSystem::GetCameraForPlayerID(int playerID) const
{
	if (m_playerCameras[playerID] != nullptr)
	{
		return m_playerCameras[playerID];
	}
	else
	{
		return nullptr;
	}
}

void SplitScreenSystem::SetColorTargets(ColorTargetView * colorTargetView) const
{
	for (int cameraIndex = 0; cameraIndex < m_numPlayers; cameraIndex++)
	{
		m_playerCameras[cameraIndex]->SetColorTarget(colorTargetView);
	}
}
