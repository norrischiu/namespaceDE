workspace "Sample"
	configurations { "Debug", "Release" }
	startproject "Sample"
	architecture "x64"
	characterset "MBCS"
	language "C++"
	cppdialect "C++17"
	location "Build"
	platforms { "x64" }
	systemversion "10.0.16299.0"
	
-- DEngine
include("../DEngine/premake5.lua")

-- Sample
project "Sample"
	location "Build"
	kind "WindowedApp"
	defines {"_CRT_SECURE_NO_WARNINGS"}
	includedirs { "Source/", "../DEngine/Source/" }
	buildoptions { "/sdl" } -- code generation for uninitialized variable
	links { "DECore", "DERendering", "DEGame" }

	files 
	{ 
		"Source/**.h", 
		"Source/**.cpp",
		"Assets/Shader/**.hlsl",
		"Assets/Shader/**.hlsli"
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		targetdir "Bin/Debug"
		objdir "Intermediate/Debug"
		symbols "on"
		debugdir "Bin/Debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "Full"
		targetdir "Bin/Release"
		objdir "Intermediate/Release"
		debugdir "Bin/Release"
	filter {}

	filter { 'files:**.vs.hlsl' }
		buildcommands 'fxc.exe -Zi -E main -Od -T vs_5_0 -Fo $(OutputPath)../Assets/Shaders/%(Filename).cso %(FullPath)'
		buildoutputs '$(OutputPath)../Assets/Shaders/%(Filename).cso'
	filter { 'files:**.ps.hlsl' }
		buildcommands 'fxc.exe -Zi -E main -Od -T ps_5_0 -Fo $(OutputPath)../Assets/Shaders/%(Filename).cso %(FullPath)'
		buildoutputs '$(OutputPath)../Assets/Shaders/%(Filename).cso'	
	filter { 'files:**.cs.hlsl' }
		buildcommands 'fxc.exe -Zi -E main -Od -T cs_5_0 -Fo $(OutputPath)../Assets/Shaders/%(Filename).cso %(FullPath)'
		buildoutputs '$(OutputPath)../Assets/Shaders/%(Filename).cso'
	filter {}