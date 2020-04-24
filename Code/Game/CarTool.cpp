#include "Game/CarTool.hpp"
#include "Engine/Renderer/ImGUISystem.hpp"
#include "Engine/PhysXSystem/PhysXVehicleFilterShader.hpp"

//------------------------------------------------------------------------------------------------------------------------------
CarTool::CarTool()
{
	SetAllDefaults();
}

//------------------------------------------------------------------------------------------------------------------------------
CarTool::~CarTool()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::UpdateImGUICarTool()
{
	ImGui::Begin("Car Tuner");

	ImGui::Columns(3, NULL, false);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

	UpdateWheelData();
	ImGui::NextColumn();

	UpdateChassisData();
	ImGui::NextColumn();

	UpdateDifferentialData();
	ImGui::NextColumn();

	UpdateEngineData();
	ImGui::NextColumn();

	UpdateGearData();
	ImGui::NextColumn();

	ImGui::PopStyleVar();
	ImGui::End();
}

//------------------------------------------------------------------------------------------------------------------------------
physx::PxVehicleDrive4W* CarTool::MakeNewCar()
{
	//First ensure m_driveSimData is correctly setup
	m_driveSimData.setEngineData(m_engineData);
	m_driveSimData.setClutchData(m_clutchData);
	m_driveSimData.setDiffData(m_differnetialData);
	m_driveSimData.setGearsData(m_gearData);
	m_driveSimData.setAckermannGeometryData(m_ackermanGeometry);

	m_vehicleDesc.wheelMaterial = g_PxPhysXSystem->GetDefaultPxMaterial();
	m_vehicleDesc.chassisMaterial = g_PxPhysXSystem->GetDefaultPxMaterial();

	return g_PxPhysXSystem->CreateCustomVehicle4W(m_vehicleDesc, m_driveSimData);
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::SetAllDefaults()
{
	SetDefaultVehicleDesc();
	SetDefaultDifferentialData();
	SetDefaultEngineData();
	SetDefaultGearData();
	SetDefaultClutchData();

}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::SetDefaultVehicleDesc()
{
	m_vehicleDesc.chassisMass = 1500.f;
	m_vehicleDesc.chassisDims = PxVec3(2.5f, 1.20f, 5.0f);

	m_vehicleDesc.chassisMOI =
		PxVec3((m_vehicleDesc.chassisDims.y*m_vehicleDesc.chassisDims.y + m_vehicleDesc.chassisDims.z*m_vehicleDesc.chassisDims.z)*m_vehicleDesc.chassisMass / 12.0f,
		(m_vehicleDesc.chassisDims.x*m_vehicleDesc.chassisDims.x + m_vehicleDesc.chassisDims.z*m_vehicleDesc.chassisDims.z)*0.8f*m_vehicleDesc.chassisMass / 12.0f,
			(m_vehicleDesc.chassisDims.x*m_vehicleDesc.chassisDims.x + m_vehicleDesc.chassisDims.y*m_vehicleDesc.chassisDims.y)*m_vehicleDesc.chassisMass / 12.0f);
	m_vehicleDesc.chassisCMOffset = PxVec3(0.0f, -m_vehicleDesc.chassisDims.y*0.5f + 0.65f, 0.25f);

	//Set up the wheel mass, radius, width, moment of inertia, and number of wheels.
	//Moment of inertia is just the moment of inertia of a cylinder.
	m_vehicleDesc.wheelMass = 20.0f;
	m_vehicleDesc.wheelRadius = 0.5f;
	m_vehicleDesc.wheelWidth = 0.4f;
	m_vehicleDesc.wheelMOI = 0.5f*m_vehicleDesc.wheelMass*m_vehicleDesc.wheelRadius*m_vehicleDesc.wheelRadius;
	m_vehicleDesc.numWheels = 4;

	m_chassisSimFilter = PxFilterData(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);
	m_wheelSimFilter = PxFilterData(COLLISION_FLAG_WHEEL, COLLISION_FLAG_WHEEL_AGAINST, PxPairFlag::eDETECT_CCD_CONTACT | PxPairFlag::eMODIFY_CONTACTS, 0);

	m_vehicleDesc.chassisSimFilterData = m_chassisSimFilter;
	m_vehicleDesc.wheelSimFilterData = m_wheelSimFilter;

	m_vehicleDesc.actorUserData = &m_actorUserData;
	m_vehicleDesc.shapeUserDatas = m_shapeUserData;
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::SetDefaultDifferentialData()
{
	m_differnetialData.mFrontRearSplit = 0.45f;
	m_differnetialData.mFrontLeftRightSplit = 0.5f;
	m_differnetialData.mRearLeftRightSplit = 0.5f;
	m_differnetialData.mCentreBias = 1.3f;
	m_differnetialData.mFrontBias = 1.3f;
	m_differnetialData.mRearBias = 1.3f;
	m_differnetialData.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::SetDefaultEngineData()
{
	m_engineData.mMOI = 1.0f;
	m_engineData.mPeakTorque = 500.f;
	m_engineData.mMaxOmega = 600.f;
	m_engineData.mDampingRateFullThrottle = 0.15f;
	m_engineData.mDampingRateZeroThrottleClutchEngaged = 2.0f;
	m_engineData.mDampingRateZeroThrottleClutchDisengaged = 0.35f;

	m_engineData.mTorqueCurve.addPair(0.0f, 0.8f);
	m_engineData.mTorqueCurve.addPair(0.33f, 1.0f);
	m_engineData.mTorqueCurve.addPair(1.0f, 0.8f);

	//These values are set in the setEngineData function on vehicle create
	//m_engineData.mRecipMOI = 1.0f / m_engineData.mMOI;
	//m_engineData.mRecipMaxOmega = 1.0f / m_engineData.mMaxOmega;
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::SetDefaultGearData()
{
	m_gearData.mNbRatios = 7;	//Reverse, Neutral, 1-5
	m_gearData.mFinalRatio = 4.f;
	m_gearData.mSwitchTime = 0.1f;

	m_gearData.mRatios[PxVehicleGearsData::eREVERSE] = -4.0f;
	m_gearData.mRatios[PxVehicleGearsData::eNEUTRAL] = 0.0f;
	m_gearData.mRatios[PxVehicleGearsData::eFIRST] = 4.0f;
	m_gearData.mRatios[PxVehicleGearsData::eSECOND] = 2.0f;
	m_gearData.mRatios[PxVehicleGearsData::eTHIRD] = 1.5f;
	m_gearData.mRatios[PxVehicleGearsData::eFOURTH] = 1.1f;
	m_gearData.mRatios[PxVehicleGearsData::eFIFTH] = 1.0f;

	for (PxU32 i = PxVehicleGearsData::eSIXTH; i < PxVehicleGearsData::eGEARSRATIO_COUNT; ++i)
		m_gearData.mRatios[i] = 0.f;
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::SetDefaultClutchData()
{
	m_clutchData.mStrength = 10.f;
	m_clutchData.mEstimateIterations = 5;
	m_clutchData.mAccuracyMode = PxVehicleClutchAccuracyMode::eBEST_POSSIBLE;
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::UpdateWheelData()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("Wheel Editor", ImVec2(m_defaultColumnWidth, m_defaultColumnHeight), true, window_flags);

	ImGui::Text("Wheel Editor");

	//Wheel width
	ImGui::DragFloat("Wheel Width: ", &m_vehicleDesc.wheelWidth);

	//Wheel mass
	ImGui::DragFloat("Wheel Mass", &m_vehicleDesc.wheelMass);

	//Wheel Radius
	ImGui::DragFloat("Wheel Radius", &m_vehicleDesc.wheelRadius);

	ImGui::EndChild();
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::UpdateChassisData()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("Chassis Editor", ImVec2(m_defaultColumnWidth, m_defaultColumnHeight), true, window_flags);

	ImGui::Text("Chassis Editor");

	//chassis mass
	ImGui::DragFloat("Chassis Mass: ", &m_vehicleDesc.chassisMass);

	//chassis extents
	float dimensions[3];
	dimensions[0] = m_vehicleDesc.chassisDims.x;
	dimensions[1] = m_vehicleDesc.chassisDims.y;
	dimensions[2] = m_vehicleDesc.chassisDims.z;
	ImGui::DragFloat3("Chassis Dimensions: ", dimensions);
	m_vehicleDesc.chassisDims.x = dimensions[0];
	m_vehicleDesc.chassisDims.y = dimensions[1];
	m_vehicleDesc.chassisDims.z = dimensions[2];

	ImGui::EndChild();
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::UpdateDifferentialData()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("Diff Editor", ImVec2(m_defaultColumnWidth, m_defaultColumnHeight), true, window_flags);

	ImGui::Text("Differential Editor");

	//Differential Front and rear split
	ImGui::DragFloat("Diff Front Rear Split: ", &m_differnetialData.mFrontRearSplit);
	ImGui::DragFloat("Diff Front Left-Right Split: ", &m_differnetialData.mFrontLeftRightSplit);
	ImGui::DragFloat("Diff Rear Left-Right Split: ", &m_differnetialData.mRearLeftRightSplit);
	ImGui::DragFloat("Diff Center Bias: ", &m_differnetialData.mCentreBias);
	ImGui::DragFloat("Diff Front Bias: ", &m_differnetialData.mFrontBias);
	ImGui::DragFloat("Diff Rear Bias: ", &m_differnetialData.mRearBias);
	
	const char* items[] = { "LS_4WD", "LS_FWD", "LS_RWD", "O_4WD", "O_FWD", "O_RWD" };
	static int item_current = 0;
	ImGui::Combo("Diff Type", &item_current, items, IM_ARRAYSIZE(items));
// 	for (int i = 0; i < IM_ARRAYSIZE(items); i++)
// 	{
// 		bool is_selected = (item_current == i);
// 		if (ImGui::Selectable(items[i], is_selected))
// 			item_current = i;
// 		if (is_selected)
// 			ImGui::SetItemDefaultFocus();
// 	}
	m_differnetialData.mType = (PxVehicleDifferential4WData::Enum)item_current;

	ImGui::EndChild();
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::UpdateEngineData()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("Engine Editor", ImVec2(m_defaultColumnWidth, m_defaultColumnHeight), true, window_flags);

	ImGui::Text("Engine Editor");

	ImGui::DragFloat("Moment of Inertia: ", &m_engineData.mMOI);
	ImGui::DragFloat("Peak Torque: ", &m_engineData.mPeakTorque);
	ImGui::DragFloat("Max Omega: ", &m_engineData.mMaxOmega);
	ImGui::DragFloat("Damping Rate Full Throttle: ", &m_engineData.mDampingRateFullThrottle);
	ImGui::DragFloat("Damping 0 throttle (Clutch engaged): ", &m_engineData.mDampingRateZeroThrottleClutchEngaged);
	ImGui::DragFloat("Damping 0 throttle (Clutch disengaged): ", &m_engineData.mDampingRateZeroThrottleClutchDisengaged);
	//Figure out a way to show torque curves on a graph! CurveEditor not working correctly

// 	float* floatArray = new float[m_engineData.mTorqueCurve.mNbDataPairs * 2];
// 	int pointCount = 0;
// 
// 	float* dataPairs = m_engineData.mTorqueCurve.mDataPairs;
// 
// 	for (int torqueCurveIndex = 0; torqueCurveIndex < m_engineData.mTorqueCurve.mNbDataPairs * 2; torqueCurveIndex++)
// 	{
// 		floatArray[torqueCurveIndex] = m_engineData.mTorqueCurve.getX(torqueCurveIndex);
// 		torqueCurveIndex++;
// 		pointCount++;
// 		
// 		floatArray[torqueCurveIndex] = m_engineData.mTorqueCurve.getY(torqueCurveIndex);
// 		pointCount++;
// 	}
// 
// 	int newCount = 0;
// 	g_ImGUI->CurveEditor("Engine Torque Curve", floatArray, pointCount, ImVec2(100.f, 100.f), &newCount);

	ImGui::EndChild();
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::UpdateGearData()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("Gear Editor", ImVec2(m_defaultColumnWidth, m_defaultColumnHeight), true, window_flags);

	ImGui::Text("Gear Editor");

	ImGui::DragFloat("Final Ratio: ", &m_gearData.mFinalRatio);
	ImGui::DragFloat("Switch Time: ", &m_gearData.mSwitchTime);

	ImGui::Text("Gear Ratios from Reverse to Fifth gears");
	ImGui::DragFloat("Ratio Reverse: ", &m_gearData.mRatios[PxVehicleGearsData::eREVERSE]);
	ImGui::DragFloat("Ratio Neutral: ", &m_gearData.mRatios[PxVehicleGearsData::eNEUTRAL]);
	ImGui::DragFloat("Ratio First: ", &m_gearData.mRatios[PxVehicleGearsData::eFIRST]);
	ImGui::DragFloat("Ratio Second: ", &m_gearData.mRatios[PxVehicleGearsData::eSECOND]);
	ImGui::DragFloat("Ratio Third: ", &m_gearData.mRatios[PxVehicleGearsData::eTHIRD]);
	ImGui::DragFloat("Ratio Fourth: ", &m_gearData.mRatios[PxVehicleGearsData::eFOURTH]);
	ImGui::DragFloat("Ratio Fifth: ", &m_gearData.mRatios[PxVehicleGearsData::eFIFTH]);

	ImGui::EndChild();
}

//------------------------------------------------------------------------------------------------------------------------------
void CarTool::UpdateClutchData()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
	ImGui::BeginChild("Clutch Editor", ImVec2(m_defaultColumnWidth, m_defaultColumnHeight), true, window_flags);

	ImGui::DragFloat("Clutch Strength: ", &m_clutchData.mStrength);

	ImGui::EndChild();
}

