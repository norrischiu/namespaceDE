#pragma once

// Engine
#include <DERendering/Device/RenderDevice.h>
#include <DECore/Math/simdmath.h>

namespace DE
{

class DrawCommandList;
class FrameData;

class ForwardPass
{
	struct Data final
	{
		GraphicsPipelineState pso;
		RootSignature rootSignature;
		Texture depth;
		Texture irradianceMap;
		uint32_t backBufferIndex = 0;

		ConstantBufferView vsCbv;
		ConstantBufferView psCbv;

		RenderDevice* pDevice;
	};

	struct PerLightCBuffer
	{
		Vector3 lightPos;
		Vector3 eyePos;
	};
	struct PerObjectCBuffer
	{
		Matrix4 world;
		Matrix4 wvp;
	};


public:
	ForwardPass() = default;

	void Setup(RenderDevice* renderDevice, Texture& irradianceMap);
	void Execute(DrawCommandList& commandList, const FrameData& frameData);

	// temp
	Texture* GetDepth()
	{
		return &data.depth;
	}

private:
	Data data;
};

} // namespace DE