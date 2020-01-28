#pragma once

// Cpp
#include <assert.h>
#include <stdint.h>
// Engine
#include <DERendering/DataType/GraphicsNativeType.h>
#include <DERendering/DataType/GraphicsViewType.h>
#include <DERendering/DataType/GraphicsResourceType.h>
#include <DERendering/DataType/GraphicsDataType.h>

inline std::size_t Align(std::size_t size, std::size_t align)
{
	return size + align - 1 & ~(align - 1);
}