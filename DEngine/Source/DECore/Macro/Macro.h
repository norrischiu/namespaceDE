#pragma once

#include <windows.h> 

#if DLL_EXPORT
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif