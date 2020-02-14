#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/PhysXSystem/PhysXSystem.hpp"

class WayPoint
{
public:

	WayPoint();
	~WayPoint();

	//Setup the way point 
	void SetupWaypoint(const Vec3& waypointPosition, const Vec3& wayPointHalfExtents);

private:

	float m_timeAtPass = 0.f;
};
