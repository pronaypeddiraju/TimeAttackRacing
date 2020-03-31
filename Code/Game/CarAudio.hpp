#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include "Game/CarController.hpp"

class CarAudio
{
public:
	CarAudio(CarController* controllerRef);
	~CarAudio();

	void			SetAllSoundIDs(const std::vector<SoundID>& soundIDs );
	void			InitializeFromPaths(const std::vector<std::string>& audioFilePaths);

	void			Update();

private:
	void			UpdateRPMBased();
	void			UpdateGearRatioBased();
	void			UpdateSimplex();
	void			UpdateSimplexMultiTrack();

private:
	SoundID			m_carSoundIDs[16];
	SoundPlaybackID	m_soundPlaybackIDs[16];
	SoundPlaybackID	m_tempPlaybackID;

	const float		m_MIN_RPM_TABLE[16] = { 50.f, 75.f, 112.f, 166.9f, 222.4f, 278.3f, 333.5f, 388.2f, 435.5f, 483.3f, 538.4f, 594.3f, 643.6f, 692.8f, 741.9f, 790.0f };
	const float		m_NORMAL_RPM_TABLE[16] = { 72.0f, 93.0f, 155.9f, 202.8f, 267.0f, 314.5f, 377.4f, 423.9f, 472.1f, 519.4f, 582.3f, 631.3f, 680.8f, 729.4f, 778.8f, 826.1f };
	const float		m_MAX_RPM_TABLE[16] = { 92.0f, 136.0f, 182.9f, 247.4f, 294.3f, 357.5f, 403.6f, 452.5f, 499.3f, 562.5f, 612.3f, 661.6f, 708.8f, 758.9f, 806.0f, 1000.0f };

	const float		m_PITCHING_TABLE[16] = { 0.12f, 0.12f, 0.12f, 0.12f, 0.11f, 0.10f, 0.09f, 0.08f, 0.06f, 0.06f, 0.06f, 0.06f, 0.06f, 0.06f, 0.06f, 0.06f };

	CarController*	m_carControllerRef = nullptr;
	float			m_startFrequency = 0.f;

	float			m_rangeDivider = 2.f;
	int				m_numFiles = 0;
};