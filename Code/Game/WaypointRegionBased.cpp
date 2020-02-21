#include "Game/WaypointRegionBased.hpp"
//Engine Systems
#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB3.hpp"

//------------------------------------------------------------------------------------------------------------------------------
WaypointRegionBased::WaypointRegionBased()
{

}

//------------------------------------------------------------------------------------------------------------------------------
WaypointRegionBased::WaypointRegionBased(const Vec3& wayPointPosition, const Vec3& halfExtents, uint waypointIndex)
{
	m_position = wayPointPosition;
	m_halfExtents = halfExtents;

	m_shape = AABB3(m_position - m_halfExtents, m_position + m_halfExtents);
	
	m_waypointIndex = waypointIndex;
}

//------------------------------------------------------------------------------------------------------------------------------
WaypointRegionBased::WaypointRegionBased(const Vec3& waypointPosition, const AABB3& waypointShape, uint waypointIndex)
{
	m_position = waypointPosition;
	m_shape = waypointShape;
	m_waypointIndex = waypointIndex;
}

//------------------------------------------------------------------------------------------------------------------------------
bool WaypointRegionBased::HasPointCrossedWaypoint(const Vec3& pointToCheck)
{
	return m_shape.IsPointInsideAABB3(pointToCheck);
}

//------------------------------------------------------------------------------------------------------------------------------
const Vec3& WaypointRegionBased::GetWaypointMins() const
{
	return m_shape.GetMins();
}

//------------------------------------------------------------------------------------------------------------------------------
const Vec3& WaypointRegionBased::GetWaypointMaxs() const
{
	return m_shape.GetMaxs();
}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointRegionBased::AssignWaypointNumber(uint numberToAssign)
{
	m_waypointIndex = numberToAssign;
}

//------------------------------------------------------------------------------------------------------------------------------
uint WaypointRegionBased::GetWaypointNumber() const
{
	return m_waypointIndex;
}

//------------------------------------------------------------------------------------------------------------------------------
WaypointRegionBased::~WaypointRegionBased()
{

}
