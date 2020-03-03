#pragma once
#include "Game/CarCamera.hpp"
#include <map>

//------------------------------------------------------------------------------------------------------------------------------
class SplitScreenSystem
{
public:

	SplitScreenSystem();
	~SplitScreenSystem();

	void		AddCarCameraForPlayer(CarCamera& camera, int playerID);
	int			GetNumPlayers() const;

	void		ComputeSplits();

	const std::map<int, CarCamera&>&	GetAllCameras() const;
	const CarCamera*					GetCameraForPlayerID(int playerID) const;

private:

	std::map<int, CarCamera&>	m_playerCameras;
	int							m_numPlayers = 0;
};