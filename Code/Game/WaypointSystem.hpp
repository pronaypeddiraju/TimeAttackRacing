#pragma once
//Engine Systems
#include "Engine/Commons/EngineCommon.hpp"
//Game Systems
#include "Game/WaypointRegionBased.hpp"
#include <vector>

//------------------------------------------------------------------------------------------------------------------------------
class WaypointSystem 
{
public:
	WaypointSystem();
	~WaypointSystem();

	void					AddNewWayPoint(const Vec3& waypointPosition, const Vec3& waypointHalfExtents, uint waypointIndex);
	uint					GetNextWaypointIndex() const;
	uint					GetCurrentWaypointIndex() const;
	const Vec3&				GetNextWaypointPosition() const;
	Matrix44				GetNextWaypointModelMatrix() const;

	uint					GetCurrentLapNumber() const;
	uint					GetMaxLapCount() const;
	void					SetMaxLapCount(uint maxLapCount);

	double					GetTotalTime() const;
	bool					AreLapsComplete() const;

	void					Startup();
	void					Update(const Vec3& carPosition);
	
	void					RenderNextWaypoint() const;
	void					DebugRenderWaypoints() const;

	void					UpdateImGUIForWaypoints();

	void					Reset();

private:
	void					SetSystemToNextWaypoint();
	void					AddTimeStampForLap();
	double					GetLastLapTime();
	double					GetAccumulatedLapTimes() const;

	void					ComputeBestLapTimeForRun();

private:
	std::vector<WaypointRegionBased> m_waypointList;
	uint					m_crossedIndex = UINT_MAX;	//This is Uint max. I'm not stupid this was on purpose
	uint					m_lapIndex = 1;
	uint					m_maxLaps = 1;

	bool					m_lapsCompleted = false;

	double					m_startTime = 0.0;

	std::vector<double>		m_timeStamps;
};
