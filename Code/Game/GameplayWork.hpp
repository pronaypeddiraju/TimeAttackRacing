#pragma once

#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/Model.hpp"
#include <string>

//------------------------------------------------------------------------------------------------------------------------------
struct ImageLoadWork
{
	ImageLoadWork(const std::string& fileName);
	~ImageLoadWork();

	std::string imageName;
	Image* image = nullptr;
};

//------------------------------------------------------------------------------------------------------------------------------
struct ModelLoadWork
{
	ModelLoadWork(const std::string& fileName);
	~ModelLoadWork();

	std::string modelName = "";
	CPUMesh* mesh = nullptr;
	Model* model = nullptr;
	std::string materialPath = "";
};