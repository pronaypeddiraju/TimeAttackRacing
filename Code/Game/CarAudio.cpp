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
		g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[audioIndex], 0.f);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::Startup()
{
	StartupSimplex();
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::Update()
{
	UpdateSimplexMultiTrack();
	//UpdateSimplex();
}

//------------------------------------------------------------------------------------------------------------------------------
const SoundID& CarAudio::GetSimplexSoundID() const
{
	return m_simplexSound;
}

//------------------------------------------------------------------------------------------------------------------------------
const SoundPlaybackID& CarAudio::GetSimplexSoundPlaybackID() const
{
	return m_simplexPlaybackID;
}

//------------------------------------------------------------------------------------------------------------------------------
const SoundID* CarAudio::GetSoundIDs(int& size) const
{
	size = m_numFiles;
	return m_carSoundIDs;
}

//------------------------------------------------------------------------------------------------------------------------------
const SoundPlaybackID* CarAudio::GetSoundPlaybackIDs(int& size) const
{
	size = m_numFiles;
	return m_soundPlaybackIDs;
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::SetSimplexSoundID(const SoundID& soundID)
{
	m_simplexSound = soundID;
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::SetSimplexSoundPlaybackID(const SoundPlaybackID& soundPlaybackID)
{
	m_simplexPlaybackID = soundPlaybackID;
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::SetSoundIDs(const SoundID* soundID, int size)
{
	SoundID* thisCarSounds = m_carSoundIDs;
	for (int i = 0; i < size; i++)
	{
		thisCarSounds = const_cast<SoundID*>(soundID);
		soundID++;
		thisCarSounds++;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::SetSoundPlaybackIDs(const SoundPlaybackID* soundPlaybackIDs, int size)
{
	SoundPlaybackID* thisCarSoundPlaybacks = m_soundPlaybackIDs;
	for (int i = 0; i < size; i++)
	{
		thisCarSoundPlaybacks = const_cast<SoundPlaybackID*>(soundPlaybackIDs);
		soundPlaybackIDs++;
		thisCarSoundPlaybacks++;
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::SetNewPlaybackIDs()
{
	for (int audioIndex = 0; audioIndex < m_numFiles; audioIndex++)
	{
		m_soundPlaybackIDs[audioIndex] = g_audio->PlayAudio(audioIndex, true);
		g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[audioIndex], 0.f);
	}
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::StartupSimplex()
{
	SoundID id = g_audio->CreateOrGetSound(m_engineSoundPath);
	m_simplexSound = id;
	m_simplexPlaybackID = g_audio->PlayAudio(m_simplexSound, true);
	g_audio->SetSoundPlaybackVolume(m_simplexPlaybackID, 0.f);

	m_shiftSoundID = g_audio->CreateOrGetSound(m_shiftSound);
	m_shiftPlaybackID = g_audio->PlayAudio(m_shiftSoundID, true);
	g_audio->SetSoundPlaybackVolume(m_shiftPlaybackID, 0.f);

	DebuggerPrintf("\n\n Frequency = %f", m_startFrequency);
}

//------------------------------------------------------------------------------------------------------------------------------
void CarAudio::UpdateRPMBased()
{
	//Get the car's RPM

	float omega = m_carControllerRef->GetVehicle()->mDriveSimData.getEngineData().mMaxOmega;
	float currentSpeed = m_carControllerRef->GetVehicle()->mDriveDynData.getEngineRotationSpeed();

	float radiansPerSecond = m_carControllerRef->GetVehicle()->mDriveDynData.getEngineRotationSpeed() * 60 * 0.5f / PxPi;
	float maxRadsPerSecond = m_carControllerRef->GetVehicle()->mDriveSimData.getEngineData().mMaxOmega * 60 * 0.5f / PxPi;
	float RPM = PhysXSystem::GetRadiansPerSecondToRotationsPerMinute(radiansPerSecond) * (1000.f / maxRadsPerSecond);
	float audioRatio = currentSpeed / currentSpeed * 1000.f;
	RPM = RPM * m_carControllerRef->GetVehicle()->mDriveDynData.getCurrentGear();

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

	//float timeSinceLastSwitch = vehicleRef->mDriveDynData.mGearSwitchTime;

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
		float enginePitch = (engineSpeed - gearMinValue) / (gearMaxValue - gearMinValue);
		g_audio->SetSoundPlaybackSpeed(m_tempPlaybackID, enginePitch);
		//DebuggerPrintf("\n Engine Speed: %f", engineSpeed);	//Upto around 1000
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

	if (currentGear != 1)
	{
		g_audio->SetSoundPlaybackSpeed(m_simplexPlaybackID, ratio);
		g_audio->SetSoundPlaybackVolume(m_simplexPlaybackID, 1.f);
	}
	else
	{
		g_audio->SetSoundPlaybackVolume(m_shiftPlaybackID, 0.1f);
	}

	if (currentGear != m_carControllerRef->GetVehicle()->mDriveDynData.getTargetGear() && !m_playedShift)	
	{
		m_playedShift = true;
		//Play shift sound once
		g_audio->SetSoundPlaybackVolume(m_shiftPlaybackID, 0.1f);
		g_audio->SetSoundPlaybackSpeed(m_shiftPlaybackID, 1.f);
		m_playedShift = false;
	}
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
		if ((int)currentGear == i)
		{
			g_audio->SetSoundPlaybackSpeed(m_soundPlaybackIDs[i], ratio);
			g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[i], 0.5f);

			if (i > 1)
			{
				//Add some damping for the previous sound track
				g_audio->SetSoundPlaybackSpeed(m_soundPlaybackIDs[i - 1], 1 - ratio);
				g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[i], 1.f);
			}

			for (int j = 0; j < m_numFiles; j++)
			{
				if (j != i && j != i - 1)
				{
					g_audio->SetSoundPlaybackSpeed(m_soundPlaybackIDs[j], 0.f);
					g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[j], 0.f);
				}
			}
		}
		else if(m_carControllerRef->IsControlReleased())
		{
			for (int j = 0; j < m_numFiles; j++)
			{
				g_audio->SetSoundPlaybackSpeed(m_soundPlaybackIDs[j], 0.f);
				g_audio->SetSoundPlaybackVolume(m_soundPlaybackIDs[j], 0.f);
			}
		}
	}

	//DebuggerPrintf("\n Current Gear: %u", currentGear);

// 	if (currentGear != m_carControllerRef->GetVehicle()->mDriveDynData.getTargetGear() && !m_playedShift)
// 	{
// 		m_playedShift = true;
// 		//Play shift sound once
// 		m_shiftPlaybackID = g_audio->PlayAudio(m_shiftSoundID);
// 		g_audio->SetSoundPlaybackVolume(m_shiftPlaybackID, 0.1f);
// 		g_audio->SetSoundPlaybackSpeed(m_shiftPlaybackID, 1.f);
// 		m_playedShift = false;
// 	}

}

