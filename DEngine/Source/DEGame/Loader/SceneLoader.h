#pragma once

// Engine
#include <DEGame/DEGame.h>
#include <DEGame/Collection/Scene.h>
// Cpp
#include <string>


namespace DE 
{

class RenderDevice;
class CopyCommandList;

class SceneLoader
{
public:

	void Init(RenderDevice* device);
	void SetRootPath(const char* path);
	void Load(const char* sceneName, Scene& scene);

private:

	std::string m_sRootPath;
	RenderDevice* m_pRenderDevice;
};

}