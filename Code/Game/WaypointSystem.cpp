#include "Game/WaypointSystem.hpp"
//Engine Systems
#include "Engine/Math/Vertex_Lit.hpp"   
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/NamedProperties.hpp"

//------------------------------------------------------------------------------------------------------------------------------
WaypointSystem::WaypointSystem()
{

}

//------------------------------------------------------------------------------------------------------------------------------
WaypointSystem::~WaypointSystem()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointSystem::AddNewWayPoint(const Vec3& waypointPosition, const Vec3& waypointHalfExtents, uint waypointIndex)
{
	m_waypointList.emplace_back(waypointPosition, waypointHalfExtents, waypointIndex);
}

//------------------------------------------------------------------------------------------------------------------------------
uint WaypointSystem::GetNextWaypointIndex() const
{
	uint nextIndex = m_crossedIndex + 1;
	if (nextIndex < m_waypointList.size())
	{
		return nextIndex;
	}
	else
	{
		return 0;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
uint WaypointSystem::GetCurrentWaypointIndex() const
{
	return m_crossedIndex;
}

//------------------------------------------------------------------------------------------------------------------------------
uint WaypointSystem::GetCurrentLapNumber() const
{
	return m_lapIndex;
}

//------------------------------------------------------------------------------------------------------------------------------
uint WaypointSystem::GetMaxLapCount() const
{
	return m_maxLaps;
}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointSystem::SetMaxLapCount(uint maxLapCount)
{
	m_maxLaps = maxLapCount;
}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointSystem::Startup()
{
	m_startTime = GetCurrentTimeSeconds();
}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointSystem::Update(const Vec3& carPosition)
{
	if (m_lapsCompleted)
		return;

	std::vector<WaypointRegionBased>::iterator waypointItr;
	waypointItr = m_waypointList.begin();

	while (waypointItr != m_waypointList.end())
	{
		bool result = waypointItr->HasPointCrossedWaypoint(carPosition);

		if (result)
		{
			//Check if this is the valid next way point
			uint wayPointIndex = waypointItr->GetWaypointNumber();
			if (wayPointIndex == GetNextWaypointIndex())
			{
				g_devConsole->PrintString(Rgba::YELLOW, "Reached Next Waypoint");
				//Call update waypoint and return from here
				SetSystemToNextWaypoint();
			}
		}

		waypointItr++;
	}

}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointSystem::RenderNextWaypoint() const
{
	if (m_lapsCompleted)
		return;

	const WaypointRegionBased* nextWaypoint = &m_waypointList[GetNextWaypointIndex()];

	CPUMesh boxMesh;
	CPUMeshAddCube(&boxMesh, AABB3(nextWaypoint->GetWaypointMins(), nextWaypoint->GetWaypointMaxs()), Rgba::GREEN);
	GPUMesh mesh = GPUMesh(g_renderContext);
	mesh.CreateFromCPUMesh<Vertex_Lit>(&boxMesh, GPU_MEMORY_USAGE_STATIC);
	g_renderContext->DrawMesh(&mesh);
}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointSystem::DebugRenderWaypoints() const
{
	CPUMesh boxMesh;
	GPUMesh mesh = GPUMesh(g_renderContext);

	std::vector<WaypointRegionBased>::const_iterator waypointItr = m_waypointList.begin();
	while (waypointItr != m_waypointList.end())
	{
		boxMesh.Clear();

		CPUMeshAddCube(&boxMesh, AABB3(waypointItr->GetWaypointMins(), waypointItr->GetWaypointMaxs()), Rgba::GREEN);
		mesh.CreateFromCPUMesh<Vertex_Lit>(&boxMesh, GPU_MEMORY_USAGE_STATIC);
		g_renderContext->DrawMesh(&mesh);

		waypointItr++;
	}

}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointSystem::UpdateImGUIForWaypoints()
{
	//Create the actual ImGUI widget
	ImGui::Begin("PhysX Scene Controls");

	ImGui::Text("Number of Waypoints in System : %d", m_waypointList.size());
	ImGui::Text("Max Number of laps allowed for this track: %d", m_maxLaps);

	ImGui::End();
}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointSystem::SetSystemToNextWaypoint()
{
	//Increase m_crossedIndex;
	m_crossedIndex += 1;

	if (m_crossedIndex == m_waypointList.size() - 1)
	{
		g_devConsole->PrintString(Rgba::ORGANIC_PURPLE, "Entered the next Lap");
		AddTimeStampForLap();
		std::string printString = "Time Taken: " + ToString(m_timeStamps[m_timeStamps.size() - 1]);
		g_devConsole->PrintString(Rgba::GREEN, printString);
		m_crossedIndex = UINT_MAX;
		m_lapIndex += 1;
	}

	if (m_lapIndex > m_maxLaps)
	{
		g_devConsole->PrintString(Rgba::GREEN, "Completed Race");

		m_lapIndex = m_maxLaps; //Get it back to maxLaps just so that our UI doesn't say lap 4 of 3
		m_lapsCompleted = true;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointSystem::AddTimeStampForLap()
{
	m_timeStamps.push_back(GetAccumulatedLapTimes());
}

//------------------------------------------------------------------------------------------------------------------------------
double WaypointSystem::GetAccumulatedLapTimes() const
{
	std::vector<double>::const_iterator timeItr = m_timeStamps.begin();
	if (timeItr == m_timeStamps.end())
	{
		return GetCurrentTimeSeconds() - m_startTime;
	}
	else
	{
		double accumulatedTime = 0.0;

		while (timeItr != m_timeStamps.end())
		{
			accumulatedTime += *timeItr;

			timeItr++;
		}

		return GetCurrentTimeSeconds() - m_startTime + accumulatedTime;
	}
}