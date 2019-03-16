#pragma once

// Engine
#include <DEGame/DEGame.h>
// Cpp
#include <string>


namespace DE 
{

class RenderDevice;
class CopyCommandList;

class SceneLoader
{
public:

	void Init(RenderDevice& device);
	void SetRootPath(const char* path);
	void Load(const char* sceneName);

private:

	std::string m_sRootPath;
	RenderDevice* m_pRenderDevice;
	CopyCommandList* m_pCopyCommandList;
};

}