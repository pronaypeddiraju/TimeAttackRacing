#pragma once
#include "Game/CarCamera.hpp"
#include <array>

enum eSplitMode
{
	PREFER_VERTICAL_SPLIT,
	PREFER_HORIZONTAL_SPLIT
};

//------------------------------------------------------------------------------------------------------------------------------
class SplitScreenSystem
{
public:

	friend class Game;

	SplitScreenSystem();
	~SplitScreenSystem();

	void		AddCarCameraForPlayer(CarCamera* camera, int playerID);
	int			GetNumPlayers() const;
	void		SetNumPlayers(int numPlayers);

	void		ComputeViewPortSplits(eSplitMode splitMode = PREFER_VERTICAL_SPLIT) const;

	const std::array<CarCamera*, 4>&	GetAllCameras() const;
	const CarCamera*					GetCameraForPlayerID(int playerID) const;


	void		SetColorTargets(ColorTargetView * colorTargetView) const;
private:

	//Vertical Split ratios for 2P
	const static Vec2	VERTICAL_SPLIT_2P_FIRST_MIN;
	const static Vec2	VERTICAL_SPLIT_2P_FIRST_MAX;
	const static Vec2	VERTICAL_SPLIT_2P_SECOND_MIN;
	const static Vec2	VERTICAL_SPLIT_2P_SECOND_MAX;

	//Horizontal Split ratios for 2P
	const static Vec2	HORIZONTAL_SPLIT_2P_FIRST_MIN;
	const static Vec2	HORIZONTAL_SPLIT_2P_FIRST_MAX;
	const static Vec2	HORIZONTAL_SPLIT_2P_SECOND_MIN;
	const static Vec2	HORIZONTAL_SPLIT_2P_SECOND_MAX;

	//Vertical Split ratios for 3P
	const static Vec2	VERTICAL_SPLIT_3P_FIRST_MIN;
	const static Vec2	VERTICAL_SPLIT_3P_FIRST_MAX;
	const static Vec2	VERTICAL_SPLIT_3P_SECOND_MIN;
	const static Vec2	VERTICAL_SPLIT_3P_SECOND_MAX;
	const static Vec2	VERTICAL_SPLIT_3P_THIRD_MIN;
	const static Vec2	VERTICAL_SPLIT_3P_THIRD_MAX;
	
	//Horizontal Split ratios for 3P
	const static Vec2	HORIZONTAL_SPLIT_3P_FIRST_MIN;
	const static Vec2	HORIZONTAL_SPLIT_3P_FIRST_MAX;
	const static Vec2	HORIZONTAL_SPLIT_3P_SECOND_MIN;
	const static Vec2	HORIZONTAL_SPLIT_3P_SECOND_MAX;
	const static Vec2	HORIZONTAL_SPLIT_3P_THIRD_MIN;
	const static Vec2	HORIZONTAL_SPLIT_3P_THIRD_MAX;

	//Vertical Split ratios for 4P
	const static Vec2	VERTICAL_SPLIT_4P_FIRST_MIN;
	const static Vec2	VERTICAL_SPLIT_4P_FIRST_MAX;
	const static Vec2	VERTICAL_SPLIT_4P_SECOND_MIN;
	const static Vec2	VERTICAL_SPLIT_4P_SECOND_MAX;
	const static Vec2	VERTICAL_SPLIT_4P_THIRD_MIN;
	const static Vec2	VERTICAL_SPLIT_4P_THIRD_MAX;
	const static Vec2	VERTICAL_SPLIT_4P_FOURTH_MIN;
	const static Vec2	VERTICAL_SPLIT_4P_FOURTH_MAX;

	//Horizontal Split ratios for 4P
	const static Vec2	HORIZONTAL_SPLIT_4P_FIRST_MIN;
	const static Vec2	HORIZONTAL_SPLIT_4P_FIRST_MAX;
	const static Vec2	HORIZONTAL_SPLIT_4P_SECOND_MIN;
	const static Vec2	HORIZONTAL_SPLIT_4P_SECOND_MAX;
	const static Vec2	HORIZONTAL_SPLIT_4P_THIRD_MIN;
	const static Vec2	HORIZONTAL_SPLIT_4P_THIRD_MAX;
	const static Vec2	HORIZONTAL_SPLIT_4P_FOURTH_MIN;
	const static Vec2	HORIZONTAL_SPLIT_4P_FOURTH_MAX;

	//Player and Camera data
	std::array<CarCamera*, 4>	m_playerCameras = {nullptr, nullptr, nullptr, nullptr };	//We know we won't have more than 4 so this is fixed size
	int							m_numPlayers = 0;

	eSplitMode					m_defaultSplitMode = PREFER_VERTICAL_SPLIT;
};