#pragma once
#include "Engine/Commons/EngineCommon.hpp"

constexpr float CAMERA_SHAKE_REDUCTION_PER_SECOND = 1.f;
constexpr float MAX_SHAKE = 2.0f;

constexpr float DEVCONSOLE_LINE_HEIGHT = 12.0f;

constexpr float CLIENT_ASPECT = 2.f; // We are requesting a 2:1 aspect window area

constexpr float MAX_ZOOM_STEPS = 10.f;
constexpr float MIN_ZOOM_STEPS = -10.f;

class RenderContext;
class InputSystem;
class AudioSystem;

extern RenderContext* g_renderContext;
extern InputSystem* g_inputSystem;
extern AudioSystem* g_audio;