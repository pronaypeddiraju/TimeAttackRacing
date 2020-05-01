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
const Vec3& WaypointSystem::GetNextWaypointPosition() const
{
	int index = GetNextWaypointIndex();
	return m_waypointList[index].GetWaypointPosition();
}

//------------------------------------------------------------------------------------------------------------------------------
Matrix44 WaypointSystem::GetNextWaypointModelMatrix() const
{
	Matrix44 modelMatrix = Matrix44::IDENTITY;

	modelMatrix.SetTranslation3D(GetNextWaypointPosition(), modelMatrix);

	return modelMatrix;
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
double WaypointSystem::GetTotalTime() const
{
	if (m_lapsCompleted)
	{
		return m_timeStamps[m_timeStamps.size() - 1];
	}
	else
	{
		return GetAccumulatedLapTimes();
	}
}

//------------------------------------------------------------------------------------------------------------------------------
bool WaypointSystem::AreLapsComplete() const
{
	return m_lapsCompleted;
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
	{
		//We finished the track. There are no more waypoints
		return;
	}

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

	CPUMesh postLeftMesh;
	CPUMesh postRightMesh;
	CPUMesh postTopMesh;

	Vec3 mins = nextWaypoint->GetWaypointMins();
	Vec3 maxs = nextWaypoint->GetWaypointMaxs();
	CPUMeshAddCube(&postLeftMesh, AABB3(mins - Vec3(0.25f, 0.f, 0.25f), mins + Vec3(0.25f, maxs.y * 2.f, 0.25f)), Rgba::ORGANIC_BLUE);
	CPUMeshAddCube(&postRightMesh, AABB3(Vec3(maxs.x, mins.y, maxs.z) - Vec3(0.25f, 0.f, 0.25f), Vec3(maxs.x, mins.y, maxs.z) + Vec3(0.25f, maxs.y * 2.f, 0.25f)), Rgba::ORGANIC_BLUE);

	Vec3 dir = maxs - mins;
	dir.y = 0.5f;
	CPUMeshAddCube(&postTopMesh, AABB3(Vec3(mins.x, maxs.y, mins.z), Vec3(mins.x, maxs.y, mins.z) + dir), Rgba::ORGANIC_BLUE);
	
	GPUMesh renderMeshLeftPost = GPUMesh(g_renderContext);
	renderMeshLeftPost.CreateFromCPUMesh<Vertex_Lit>(&postLeftMesh, GPU_MEMORY_USAGE_STATIC);

	GPUMesh renderMeshRightPost = GPUMesh(g_renderContext);
	renderMeshRightPost.CreateFromCPUMesh<Vertex_Lit>(&postRightMesh, GPU_MEMORY_USAGE_STATIC);
	
	GPUMesh renderMeshTopPost = GPUMesh(g_renderContext);
	renderMeshTopPost.CreateFromCPUMesh<Vertex_Lit>(&postTopMesh, GPU_MEMORY_USAGE_STATIC);

	g_renderContext->DrawMesh(&renderMeshLeftPost);
	g_renderContext->DrawMesh(&renderMeshRightPost);
	g_renderContext->DrawMesh(&renderMeshTopPost);
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
void WaypointSystem::Reset()
{
	m_lapsCompleted = false;
	m_startTime = GetCurrentTimeSeconds();
	m_timeStamps.clear();
	m_lapIndex = 1;
	m_crossedIndex = UINT_MAX;
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
	m_timeStamps.push_back(GetLastLapTime());
}

//------------------------------------------------------------------------------------------------------------------------------
double WaypointSystem::GetLastLapTime()
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
		
		return GetCurrentTimeSeconds() - m_startTime - accumulatedTime;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
double WaypointSystem::GetAccumulatedLapTimes() const
{
	std::vector<double>::const_iterator timeItr = m_timeStamps.begin();
	double accumulatedTime = 0.0;

	while (timeItr != m_timeStamps.end())
	{
		accumulatedTime += *timeItr;

		timeItr++;
	}

	if (accumulatedTime == 0.0)
	{
		//We didn't get any accumulated time yet
		accumulatedTime = GetCurrentTimeSeconds() - m_startTime;
	}

	return accumulatedTime;
}
