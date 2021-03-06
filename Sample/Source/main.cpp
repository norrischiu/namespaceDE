// main.cpp: This file contains the 'main' function. Program execution begins and ends there.

// Window
#if DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include <Windows.h>
#include <d3d12.h>
#include <tchar.h>
// Cpp
#include <sstream>
#include <iostream>
#include <chrono>
// Engine
#include <DECore/Memory/MemoryManager.h>
#include <DECore/Job/JobScheduler.h>
#include <DECore/Windows/WindowsMsgHandler.h>
#include <DERendering/Imgui/imgui.h>
#include <DEGame/Renderer/Renderer.h>

#include "SampleModel.h"

using namespace DE;

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	else if (msg == WM_PAINT)
	{
		ValidateRect(hWnd, NULL);
		return 0;
	}
	else
	{
		InvalidateRect(hWnd, NULL, TRUE);
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: wWinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, INT)
{
	constexpr uint32_t WINDOW_WIDTH = 1024;
	constexpr uint32_t WINDOW_HEIGHT = 768;
	constexpr uint32_t WINDOW_OFFSET_X = 100;
	constexpr uint32_t WINDOW_OFFSET_Y = 100;

	UNREFERENCED_PARAMETER(hInst);

	// Register the window class
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = MsgProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = _T("DE");

	RegisterClassEx(&wc);

	// calculate desired rect size with title bar and other things
	RECT rect;
	rect.left = WINDOW_OFFSET_X;
	rect.right = WINDOW_OFFSET_X + WINDOW_WIDTH;
	rect.top = WINDOW_OFFSET_Y;
	rect.bottom = WINDOW_OFFSET_Y + WINDOW_HEIGHT;
	::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the application's window
	HWND hWnd = CreateWindow(wc.lpszClassName,
		__T("DE Sample"),
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, // disable resize for now
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		wc.hInstance,
		NULL);

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	// Memory
	MemoryManager::GetInstance()->ConstructDefaultPool();

	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	uint32_t numThread = max(sysInfo.dwNumberOfProcessors / 2, 1);

	JobScheduler::Instance()->StartUp(numThread);

	Renderer::Desc desc = {};
	desc.hWnd = hWnd;
	desc.windowWidth = WINDOW_WIDTH;
	desc.windowHeight = WINDOW_HEIGHT;
	desc.backBufferCount = 2;
	Renderer* renderer = new Renderer();
	renderer->Init(desc);

	SampleModel* sample = new SampleModel();
	sample->Setup(renderer);

	// Show the window
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	ShowCursor(true);

	// Timer
	const float FPS = 60.0f;
	float elaspedTime = 0.0f;
	auto start = std::chrono::high_resolution_clock::now();

	// Main loop
	bool bQuit = false;
	while (!bQuit)
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				bQuit = true;
				break;
			}
			WindowsMsgHandler::Handle(msg.message, msg.wParam, msg.lParam);
		}

		// Inside one frame
		auto end = std::chrono::high_resolution_clock::now();
		if (elaspedTime >= 1.0f / FPS)
		{
			// imgui state set
			ImGuiIO &io = ImGui::GetIO();
			if (::GetForegroundWindow() == hWnd)
			{
				io.MousePos = ImVec2((float)Mouse::m_currState.cursorPos.x, (float)(Mouse::m_currState.cursorPos.y));
				io.MouseDown[0] = Mouse::m_currState.Buttons[0];
				io.MouseDown[1] = Mouse::m_currState.Buttons[1];
			}
			ImGui::NewFrame();

			// app update
			sample->Update(elaspedTime);

			renderer->Update(elaspedTime);
			renderer->Render();

			elaspedTime = 0.0f;
			start = end;

			// update input state
			Keyboard::Tick();
			Mouse::Tick();
		}

		elaspedTime += std::chrono::duration<float>(end - start).count();
	}

	renderer->Destruct();

	delete sample;
	delete renderer;
	sample = nullptr;
	renderer = nullptr;

	JobScheduler::Instance()->ShutDown();
	MemoryManager::GetInstance()->Destruct();

	UnregisterClass(wc.lpszClassName, wc.hInstance);
#if DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}