#pragma once

#include <DECore/DECore.h>
#include <DERendering/Device/RenderDevice.h>
#include <DERendering/DataType/GraphicsNativeType.h>
#include <DERendering/DataType/GraphicsViewType.h>
#include <DERendering/DataType/GraphicsResourceType.h>
#include <DERendering/DataType/GraphicsDataType.h>

inline std::size_t Align(std::size_t size, std::size_t align)
{
	return size + align - 1 & ~(align - 1);
}