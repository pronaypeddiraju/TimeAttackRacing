#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Commons/EngineCommon.hpp"

//------------------------------------------------------------------------------------------------------------------------------
class WaypointRegionBased
{
public:
	WaypointRegionBased();
	~WaypointRegionBased();
	explicit WaypointRegionBased(const Vec3& wayPointPosition, const Vec3& halfExtents, uint waypointIndex);
	explicit WaypointRegionBased(const Vec3& waypointPosition, const AABB3& waypointShape, uint waypointIndex);

	bool				HasPointCrossedWaypoint(const Vec3& pointToCheck);

	const Vec3&			GetWaypointMins() const;
	const Vec3&			GetWaypointMaxs() const;

	void				AssignWaypointNumber(uint numberToAssign);
	uint				GetWaypointNumber() const;

	const Vec3&			GetWaypointPosition() const;

private:
	uint		m_waypointIndex = UINT_MAX;

	Vec3		m_position = Vec3::ZERO;
	Vec3		m_halfExtents = Vec3::ZERO;
	AABB3		m_shape;
};