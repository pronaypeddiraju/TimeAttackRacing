#pragma once
//Engine systems
#include "Engine/Math/Vec3.hpp"
#include "Engine/PhysXSystem/PhysXSystem.hpp"
#include "Engine/PhysXSystem/PhysXSimulationEventCallbacks.hpp"
//Game systems

class Game;

//------------------------------------------------------------------------------------------------------------------------------
class WaypointTriggerBased : public PhysXSimulationEventCallbacks
{
public:
	WaypointTriggerBased();
	explicit WaypointTriggerBased(const Game& game);
	~WaypointTriggerBased();

	//Setup the way point 
	void					SetupWaypoint(const Vec3& waypointPosition, const Vec3& wayPointHalfExtents);

	//////////////////////////////////////////////////////////////////////////
	//Implement the PhysX simulation callbacks that you need here

	virtual void			onTrigger(PxTriggerPair* pairs, PxU32 count);

private:

	float					m_timeAtPass = 0.f;
	PxShape*				m_triggerShape = nullptr;
	PxRigidStatic*			m_thisActor = nullptr;

	const Game*				m_game = nullptr;

};
