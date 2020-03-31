#include "Game/CarAudio.hpp"
#include "Engine/Commons/EngineCommon.hpp"

//------------------------------------------------------------------------------------------------------------------------------
CarAudio::CarAudio(CarController* controllerRef)
	:	m_carControllerRef(controllerRef)
{

}

//------------------------------------------------------------------------------------------------------------------------------
CarAudio::~CarAudio()
{

}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::SetAllSoundIDs(const std::vector<SoundID>& soundIDs)
{
	for (int audioIndex = 0; audioIndex < 16; audioIndex++)
	{
		m_carSoundIDs[audioIndex] = soundIDs[audioIndex];
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::InitializeFromPaths(const std::vector<std::string>& audioFilePaths)
{
	//Load all the audio files in this path and set up their IDs
	
	m_numFiles = 16;
	if ((int)audioFilePaths.size() < m_numFiles)
		m_numFiles = (int)audioFilePaths.size();

	SoundID id;
	for (int audioIndex = 0; audioIndex < m_numFiles; audioIndex++)
	{
		id = g_audio->CreateOrGetSound(audioFilePaths[audioIndex]);
		m_carSoundIDs[audioIndex] = id;

		m_soundPlaybackIDs[audioIndex] = g_audio->PlayAudio(id, true);
	}

	//m_tempPlaybackID = g_audio->PlayAudio(m_carSoundIDs[4], true);

	DebuggerPrintf("\n\n Frequency = %f", m_startFrequency);
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::Update()
{
	UpdateSimplex();
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::UpdateRPMBased()
{
	//Get the car's RPM

	float omega = m_carControllerRef->GetVehicle()->mDriveSimData.getEngineData().mMaxOmega;
	float currentSpeed = m_carControllerRef->GetVehicle()->mDriveDynData.getEngineRotationSpeed();

	//float radiansPerSecond = m_carControllerRef->GetVehicle()->mDriveDynData.getEngineRotationSpeed() * 60 * 0.5f / PxPi;
	//float maxRadsPerSecond = m_carControllerRef->GetVehicle()->mDriveSimData.getEngineData().mMaxOmega * 60 * 0.5f / PxPi;
	//float RPM = PhysXSystem::GetRadiansPerSecondToRotationsPerMinute(radiansPerSecond) * (1000.f / maxRadsPerSecond);
	float audioRatio = currentSpeed / currentSpeed * 1000.f;
	//RPM = RPM * m_carControllerRef->GetVehicle()->mDriveDynData.getCurrentGear();

	for (int audioIndex = 0; audioIndex < 14; audioIndex++)
	{
		if (audioRatio < m_MIN_RPM_TABLE[audioIndex])
		{
			g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[audioIndex], 0.0f);
		}
		else if (audioRatio >= m_MIN_RPM_TABLE[audioIndex] && audioRatio < m_NORMAL_RPM_TABLE[audioIndex])
		{
			float Range = m_NORMAL_RPM_TABLE[audioIndex] - m_MIN_RPM_TABLE[audioIndex];
			float ReducedRPM = audioRatio - m_MIN_RPM_TABLE[audioIndex];
			float PitchValue = (ReducedRPM * m_PITCHING_TABLE[audioIndex]) / Range;

			g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[audioIndex], 0.5f);	//ReducedRPM / Range);
			g_audio->SetSoundPlaybackSpeed(m_soundPlaybackIDs[audioIndex], 1.f - m_PITCHING_TABLE[audioIndex] + PitchValue);
		}
		else if (audioRatio >= m_NORMAL_RPM_TABLE[audioIndex] && audioRatio <= m_MAX_RPM_TABLE[audioIndex])
		{
			float Range = m_MAX_RPM_TABLE[audioIndex] - m_NORMAL_RPM_TABLE[audioIndex];
			float ReducedRPM = audioRatio - m_NORMAL_RPM_TABLE[audioIndex];
			float PitchValue = (ReducedRPM * m_PITCHING_TABLE[audioIndex]) / Range;

			//m_soundPlaybackIDs[audioIndex] = g_audio->PlayAudio(m_carSoundIDs[audioIndex]);
			g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[audioIndex], 1.f);
			g_audio->SetSoundPlaybackSpeed(m_soundPlaybackIDs[audioIndex], 1.f + PitchValue);

		}
		else if (audioRatio > m_MAX_RPM_TABLE[audioIndex])
		{
			float Range = (m_MAX_RPM_TABLE[audioIndex + 1] - m_MAX_RPM_TABLE[audioIndex]) / m_rangeDivider;
			float ReducedRPM = audioRatio - m_MAX_RPM_TABLE[audioIndex];
			float PitchValue = (ReducedRPM * m_PITCHING_TABLE[audioIndex]) / Range;

			//m_soundPlaybackIDs[audioIndex] = g_audio->PlayAudio(m_carSoundIDs[audioIndex]);
			g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[audioIndex], 0.5f);		//1.f - ReducedRPM / Range);
			g_audio->SetSoundPlaybackSpeed(m_soundPlaybackIDs[audioIndex], 1.f + m_PITCHING_TABLE[audioIndex] + PitchValue);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::UpdateGearRatioBased()
{
	PxVehicleDrive4W* vehicleRef = m_carControllerRef->GetVehicle();
	PxVehicleGearsData gearData = vehicleRef->mDriveSimData.getGearsData();
	int numRatios = gearData.mNbRatios;

	float timeSinceLastSwitch = vehicleRef->mDriveDynData.mGearSwitchTime;

	for (int i = 0; i < numRatios; i++)
	{
		if (vehicleRef->mDriveDynData.getCurrentGear() < PxVehicleGearsData::eNEUTRAL)
		{
			break;
		}

		float gearMinValue = 0.f;
		float gearMaxValue = 0.f;

		if (vehicleRef->mDriveDynData.getCurrentGear() == PxVehicleGearsData::eFIRST)
		{
			gearMinValue = 0.f;
			gearMaxValue = gearData.mRatios[(int)vehicleRef->mDriveDynData.getCurrentGear()] * gearData.mFinalRatio;
		}
		else
		{
			gearMinValue = gearData.mRatios[(int)vehicleRef->mDriveDynData.getCurrentGear() - 1] * gearData.mFinalRatio;
			gearMaxValue = gearData.mRatios[(int)vehicleRef->mDriveDynData.getCurrentGear()] * gearData.mFinalRatio;
		}

		float engineSpeed = vehicleRef->mDriveDynData.getEngineRotationSpeed();
		float enginePitch = (gearMinValue) / (gearMaxValue - gearMinValue);
		g_audio->SetSoundPlaybackSpeed(m_tempPlaybackID, enginePitch);
		//DebuggerPrintf("\n Engine Speed: %f", engineSpeed);	//Upto around 1000
		DebuggerPrintf("\n Pitch : %f", gearMinValue);
	}


	//m_tempPlaybackID = g_audio->PlayAudio(m_carSoundIDs[4]);
	//g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[4], 1.f);
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::UpdateSimplex()
{
	float omega = m_carControllerRef->GetVehicle()->mDriveSimData.getEngineData().mMaxOmega;
	float currentSpeed = m_carControllerRef->GetVehicle()->mDriveDynData.getEngineRotationSpeed();

	float ratio = currentSpeed / omega;
	uint currentGear = m_carControllerRef->GetVehicle()->mDriveDynData.getCurrentGear();

	g_audio->SetSoundPlaybackSpeed(m_tempPlaybackID, ratio);

	//DebuggerPrintf("\n Pitch : %u", currentGear);
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::UpdateSimplexMultiTrack()
{
	float omega = m_carControllerRef->GetVehicle()->mDriveSimData.getEngineData().mMaxOmega;
	float currentSpeed = m_carControllerRef->GetVehicle()->mDriveDynData.getEngineRotationSpeed();

	float ratio = currentSpeed / omega;
	uint currentGear = m_carControllerRef->GetVehicle()->mDriveDynData.getCurrentGear();

	for (int i = 0; i < m_numFiles; i++)
	{
		m_soundPlaybackIDs[i] = g_audio->PlayAudio(m_carSoundIDs[currentGear], true);
		if (currentGear == i)
		{
			g_audio->SetSoundPlaybackSpeed(m_soundPlaybackIDs[i], ratio);
			g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[i], 1.f);
		}
		else
		{
			//g_audio->SetSoundPlaybackSpeed(m_soundPlaybackIDs[i], 0.f);
			g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[i], 0.f);
		}
	}
}

