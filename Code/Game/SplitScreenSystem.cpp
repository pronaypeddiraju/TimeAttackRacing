#include "Game/SplitScreenSystem.hpp"
//Engine Systems
#include "Engine/Commons/EngineCommon.hpp"

//------------------------------------------------------------------------------------------------------------------------------
SplitScreenSystem::SplitScreenSystem()
{

}

//------------------------------------------------------------------------------------------------------------------------------
SplitScreenSystem::~SplitScreenSystem()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void SplitScreenSystem::AddCarCameraForPlayer(CarCamera& camera, int playerID)
{
	//Check to ensure we don't have an entry with the same ID existing in the map
	if (m_playerCameras.find(playerID) != m_playerCameras.end())
	{
		//The playerID  already exists in the map, replace the carCamera nonetheless but yell at the user
		ERROR_RECOVERABLE("Player ID already exists in the map of playerCameras");
	}

	//We can now insert the camera into the map
	m_playerCameras.insert(std::pair<int, CarCamera&>(playerID, camera));

	m_numPlayers = (int)m_playerCameras.size();
}

//------------------------------------------------------------------------------------------------------------------------------
int SplitScreenSystem::GetNumPlayers() const
{
	return m_numPlayers;
}

//------------------------------------------------------------------------------------------------------------------------------
void SplitScreenSystem::ComputeSplits()
{
	//Check number of players and create splits accordingly

	switch (m_numPlayers)
	{
	case 1:
	{
		//We have only 1 player so we should be fine with the viewport being the whole screen
		//*m_playerCameras[0].SetViewport(Vec2::ZERO, Vec2::ONE);
	}
	break;
	case 2:
	{

	}
	break;
	case 3:
	{

	}
	break;
	case 4:
	{

	}
	break;
	default:
		break;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
const std::map<int, CarCamera&>& SplitScreenSystem::GetAllCameras() const
{
	return m_playerCameras;
}

//------------------------------------------------------------------------------------------------------------------------------
const CarCamera* SplitScreenSystem::GetCameraForPlayerID(int playerID) const
{
	std::map<int, CarCamera&>::const_iterator cameraItr = m_playerCameras.begin();

	cameraItr = m_playerCameras.find(playerID);

	if (cameraItr != m_playerCameras.end())
	{
		return &cameraItr->second;
	}
	else
	{
		return nullptr;
	}
}
