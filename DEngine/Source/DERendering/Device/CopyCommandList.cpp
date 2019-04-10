#include <DERendering/DERendering.h>
#include "CopyCommandList.h"

namespace DE
{

std::size_t Align(std::size_t size, uint32_t align)
{
	return size + align - 1 & ~(align - 1);
}

std::size_t CopyCommandList::SuballocateFromBuffer(std::size_t size, std::size_t alignment)
{
	return Align(size, alignment);
}

uint32_t CopyCommandList::Init(const GraphicsDevice& device)
{
	m_CommandList.Init(device, D3D12_COMMAND_LIST_TYPE_DIRECT); // TODO: use copy
	m_CommandList.Start();
	m_UploadBuffer.Init(device, 20 * 2048 * 2048, D3D12_HEAP_TYPE_UPLOAD);

	m_UploadBuffer.ptr->Map(0, nullptr, &m_pUploadBufferPtr);

	return 0;
}

void CopyCommandList::UploadTexture(uint8_t* source, uint32_t width, uint32_t height, uint32_t depth, DXGI_FORMAT format, Texture& destination)
{
	uint32_t numComponent = 4;

	D3D12_SUBRESOURCE_FOOTPRINT footprint = {};
	footprint.Format = format;
	footprint.Width = width;
	footprint.Height = height;
	footprint.Depth = depth;
	footprint.RowPitch = Align(width * sizeof(char) * numComponent, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	uint8_t* ptr = source;
	std::size_t offset = m_Offset;
	for (uint32_t y = 0; y < height; y++)
	{
		uint8_t* pScan = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(m_pUploadBufferPtr) + m_Offset);
		memcpy(pScan, ptr, footprint.RowPitch);
		m_Offset += footprint.RowPitch;
		ptr += footprint.RowPitch;
	}

	D3D12_TEXTURE_COPY_LOCATION dst;
	dst.pResource = destination.ptr;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0; 
	
	D3D12_TEXTURE_COPY_LOCATION src;
	src.pResource = m_UploadBuffer.ptr;
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = offset;
	src.PlacedFootprint.Footprint = footprint;

	m_CommandList.ptr->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = destination.ptr;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	m_CommandList.ptr->ResourceBarrier(1, &barrier);
}

void CopyCommandList::CopyTexture(Texture source, Texture destination)
{
	for (uint32_t i = 0; i < source.m_iNumSubresources; ++i)
	{
		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = source.ptr;
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = i;

		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = destination.ptr;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = i;

		m_CommandList.ptr->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	}
}

};