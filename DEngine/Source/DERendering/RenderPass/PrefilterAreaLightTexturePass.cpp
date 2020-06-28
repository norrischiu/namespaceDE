#include <DERendering/DERendering.h>
#include <DERendering/Device/RenderDevice.h>
#include <DERendering/DataType/GraphicsDataType.h>
#include <DERendering/RenderPass/PrefilterAreaLightTexturePass.h>
#include <DERendering/Device/DrawCommandList.h>
#include <DERendering/FrameData/FrameData.h>
#include <DERendering/RenderConstant.h>
#include <DECore/FileSystem/FileLoader.h>
#include <DECore/Job/JobScheduler.h>

namespace DE
{

bool PrefilterAreaLightTexturePass::Setup(RenderDevice *renderDevice, const Data& data)
{
	Vector<char> cs, verticalCs;
	Job *csCounter = FileLoader::LoadAsync("..\\Assets\\Shaders\\GaussianBlurHorizontal.cs.cso", cs);
	JobScheduler::Instance()->WaitOnMainThread(csCounter);
	Job *verticalCsCounter = FileLoader::LoadAsync("..\\Assets\\Shaders\\GaussianBlurVertical.cs.cso", verticalCs);
	JobScheduler::Instance()->WaitOnMainThread(verticalCsCounter);

	{
		ConstantDefinition constant = { 0, D3D12_SHADER_VISIBILITY_ALL };
		ReadOnlyResourceDefinition readOnly = { 0, 1, D3D12_SHADER_VISIBILITY_ALL };
		ReadWriteResourceDefinition readWrite = {0, 1, D3D12_SHADER_VISIBILITY_ALL};

		m_rootSignature.Add(&constant, 1);
		m_rootSignature.Add(&readOnly, 1);
		m_rootSignature.Add(&readWrite, 1);
		m_rootSignature.Finalize(renderDevice->m_Device, RootSignature::Type::Compute);
	}
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = m_rootSignature.ptr;
		desc.CS.pShaderBytecode = cs.data();
		desc.CS.BytecodeLength = cs.size();

		m_pso.Init(renderDevice->m_Device, desc);

		{
			desc.CS.pShaderBytecode = verticalCs.data();
			desc.CS.BytecodeLength = verticalCs.size();

			m_verticalPso.Init(renderDevice->m_Device, desc);
		}
	}
	{
		m_cbv.Init(renderDevice->m_Device, 256 * data.src.m_Desc.MipLevels);
	}

	m_data = data;
	m_pDevice = renderDevice;

	return true;
}

void PrefilterAreaLightTexturePass::Execute(DrawCommandList &commandList, const FrameData &frameData)
{
	commandList.SetSignature(&m_rootSignature);

	for (uint32_t i = 0; i < m_data.src.m_Desc.MipLevels; ++i)
	{
		uint32_t width = m_data.src.m_Desc.Width >> i;
		uint32_t height = m_data.src.m_Desc.Height >> i;

		struct
		{
			float2 resolution;
			float2 invResolution;
			float standardDeviation;
		} cbuf = {};
		size_t offset = max(sizeof(cbuf), 256) * i;

		cbuf.resolution = float2{ static_cast<float>(width), static_cast<float>(height) };
		cbuf.invResolution = float2{ 1.0f / width, 1.0f / height };
		cbuf.standardDeviation = 5.0f;
		m_cbv.buffer.Update(&cbuf.resolution, sizeof(cbuf), offset);
		commandList.SetConstant(0, m_cbv, offset);

		// horizontal
		commandList.UnorderedAccessBarrier(m_data.dst);
		commandList.SetReadOnlyResource(0, &ReadOnlyResource().Texture(m_data.src).Dimension(D3D12_SRV_DIMENSION_TEXTURE2D).MipRange(i, i), 1);
		commandList.SetReadWriteResource(0, &ReadWriteResource().Texture(m_data.dst).Dimension(D3D12_UAV_DIMENSION_TEXTURE2D).Mip(i), 1);
		commandList.SetPipeline(m_pso);
		commandList.Dispatch(max(1, width / 256), height, 1);

		// vertical
		commandList.UnorderedAccessBarrier(m_data.dst);
		commandList.SetReadWriteResource(0, &ReadWriteResource().Texture(m_data.dst).Dimension(D3D12_UAV_DIMENSION_TEXTURE2D).Mip(i), 1);
		commandList.SetPipeline(m_verticalPso);
		commandList.Dispatch(width, max(1, height / 256), 1);
	}

	commandList.ResourceBarrier(m_data.dst, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

} // namespace DE