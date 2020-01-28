#pragma once

// Cpp
#include <stdint.h>

inline std::size_t Align(std::size_t size, std::size_t align)
{
	return size + align - 1 & ~(align - 1);
}