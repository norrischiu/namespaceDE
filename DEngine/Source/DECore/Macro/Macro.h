#pragma once

#if DLL_EXPORT
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif