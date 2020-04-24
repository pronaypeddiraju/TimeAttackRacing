#pragma once
#include "Engine/PhysXSystem/PhysXSystem.hpp"

class CarTool
{
public:
	CarTool();
	~CarTool();

	void					UpdateImGUICarTool();
	PxVehicleDrive4W*		MakeNewCar();

private:
	void		SetAllDefaults();
	void		SetDefaultVehicleDesc();
	void		SetDefaultDifferentialData();
	void		SetDefaultEngineData();
	void		SetDefaultGearData();
	void		SetDefaultClutchData();

	void		UpdateWheelData();
	void		UpdateChassisData();
	void		UpdateDifferentialData();
	void		UpdateEngineData();
	void		UpdateGearData();
	void		UpdateClutchData();

private:

	VehicleDesc						m_vehicleDesc;
	PxVehicleDriveSimData4W			m_driveSimData;

	//Internal for DriveSimData
	PxVehicleDifferential4WData		m_differnetialData;
	PxVehicleEngineData				m_engineData;
	PxVehicleGearsData				m_gearData;
	PxVehicleClutchData				m_clutchData;
	PxVehicleAckermannGeometryData  m_ackermanGeometry;

	//Internal for VehicleDesc
	PxFilterData					m_chassisSimFilter;
	PxFilterData					m_wheelSimFilter;
	ActorUserData					m_actorUserData;
	ShapeUserData					m_shapeUserData[PX_MAX_NB_WHEELS];

	float							m_defaultColumnHeight = 150.f;
	float							m_defaultColumnWidth = 500.f;
};