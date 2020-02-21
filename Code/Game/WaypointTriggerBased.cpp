#include "Game/WaypointTriggerBased.hpp"
//Engine Systems
#include "Engine/PhysXSystem/PhysXSystem.hpp"
//Game Systems
#include "Game/Game.hpp"

//------------------------------------------------------------------------------------------------------------------------------
WaypointTriggerBased::WaypointTriggerBased(const Game& game)
{
	m_game = &game;
}

//------------------------------------------------------------------------------------------------------------------------------
WaypointTriggerBased::WaypointTriggerBased()
{

}

//------------------------------------------------------------------------------------------------------------------------------
WaypointTriggerBased::~WaypointTriggerBased()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointTriggerBased::SetupWaypoint(const Vec3& waypointPosition, const Vec3& wayPointHalfExtents)
{
	//Make a box shaped trigger using the half extents you received
	PxPhysics* PhysX = g_PxPhysXSystem->GetPhysXSDK();
	PxScene* PhysXScene = g_PxPhysXSystem->GetPhysXScene();
	PxVec3 pxPosition = g_PxPhysXSystem->VecToPxVector(waypointPosition);
	PxVec3 pxHalfExtents = g_PxPhysXSystem->VecToPxVector(wayPointHalfExtents);

	m_thisActor = PhysX->createRigidStatic(PxTransform(pxPosition));
	m_triggerShape = PxRigidActorExt::createExclusiveShape(*m_thisActor, PxBoxGeometry(pxHalfExtents.x * 0.5f, pxHalfExtents.y * 0.5f, pxHalfExtents.z * 0.5f), *g_PxPhysXSystem->GetDefaultPxMaterial());

	//Setup the correct trigger shape flags on the PxShape
	m_thisActor->getShapes(&m_triggerShape, 1);
	m_triggerShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	m_triggerShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

	PhysXScene->addActor(*m_thisActor);
}

//------------------------------------------------------------------------------------------------------------------------------
void WaypointTriggerBased::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	for (PxU32 i = 0; i < count; i++)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
			continue;

		if ((pairs[i].otherActor == m_game->GetCarActor()) && (pairs[i].triggerActor == m_thisActor))
		{
			//Do something since the car entered me
			DebuggerPrintf("Car hit the trigger");
		}
	}
}
