workspace "Sparky"
	architecture "x64"

	startproject "Testbed"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "Sparky/vendor/GLFW/include"
IncludeDir["Glad"] = "Sparky/vendor/Glad/include"
IncludeDir["ImGui"] = "Sparky/vendor/imgui"

include "Sparky/vendor/GLFW"
include "Sparky/vendor/Glad"
include "Sparky/vendor/imgui"

project "Sparky"
	location "Sparky"
	kind "SharedLib"
	language "C++"
	staticruntime "Off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "sppch.h"
	pchsource "Sparky/src/sppch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
	}

	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"

		defines
		{
			"SP_PLATFORM_WINDOWS",
			"SP_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Testbed")
		}

	filter "system:macosx"
		cppdialect "C++20"
		systemversion "latest"

		defines
		{
			"SP_PLATFORM_MACOS",
			"SP_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Testbed")
		}

	filter "configurations:Debug"
		defines { "SP_DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "SP_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "On"

project "Testbed"
	location "Testbed"
	kind "ConsoleApp"
	language "C++"
	staticruntime "Off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Sparky/vendor/spdlog/include",
		"Sparky/src"
	}

	links
	{
		"Sparky"
	}

	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"

		defines
		{
			"SP_PLATFORM_WINDOWS"
		}

	filter "system:macosx"
		cppdialect "C++20"
		systemversion "latest"

		defines
		{
			"SP_PLATFORM_MACOS"
		}

	filter "configurations:Debug"
		defines { "SP_DEBUG", "SP_ENABLE_ASSERTS" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "SP_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "On"