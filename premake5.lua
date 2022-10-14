workspace "Sparky"
	architecture "x64"
	startproject "Sparky-Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["Box2D"] = "Sparky/vendor/Box2D/include"
IncludeDir["entt"] = "Sparky/vendor/entt/include"
IncludeDir["GLFW"] = "Sparky/vendor/GLFW/include"
IncludeDir["Glad"] = "Sparky/vendor/Glad/include"
IncludeDir["glm"] = "Sparky/vendor/glm"
IncludeDir["ImGui"] = "Sparky/vendor/imgui"
IncludeDir["ImGuizmo"] = "Sparky/vendor/ImGuizmo"
IncludeDir["miniaudio"] = "Sparky/vendor/miniaudio"
IncludeDir["mono"] = "%{wks.location}/Sparky/vendor/mono/include"
IncludeDir["spdlog"] = "Sparky/vendor/spdlog/include"
IncludeDir["stb_image"] = "Sparky/vendor/stb_image"
IncludeDir["yaml_cpp"] = "Sparky/vendor/yaml-cpp/include"

LibraryDir = {}
LibraryDir["Mono"] = "%{wks.location}/Sparky/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.Mono}/mono-2.0-sgen.lib" -- dll binary must be included in build
--Library["mono"] = "%{LibraryDir.Mono}/libmono-static-sgen.lib"

group "External Dependencies"
	include "Sparky/vendor/Box2D"
	include "Sparky/vendor/GLFW"
	include "Sparky/vendor/Glad"
	include "Sparky/vendor/imgui"
	include "Sparky/vendor/yaml-cpp"
group ""


group "Core"
project "Sparky"
	location "Sparky"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "sppch.h"
	pchsource "Sparky/src/sppch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/miniaudio/miniaudio/miniaudio.h",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",

		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.yaml_cpp}",
	}

	links
	{
		"Box2D",
		"Glad",
		"GLFW",
		"ImGui",
		"%{Library.mono}",
		"opengl32.lib",
		"yaml-cpp",
	}

	filter "files:Sparky/vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SP_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
		}

	filter "configurations:Debug"
		defines "SP_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SP_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "on"


project "Sparky-ScriptCore"
	location "Sparky-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("Sparky-Editor/Resources/Scripts")
	objdir ("Sparky-Editor/Resources/Scripts/Intermediates")

	files
	{
		"%{prj.name}/Source/**.cs",
		"%{prj.name}/Properties/**.cs",
	}

	filter "configurations:Debug"
		optimize "off"
		symbols "default"

	filter "configurations:Release"
		optimize "on"
		symbols "default"

	filter "configurations:Dist"
		optimize "full"
		symbols "off"
group ""


group "Tools"
project "Sparky-Editor"
	location "Sparky-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"Sparky/src",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.spdlog}",
	}

	links
	{
		"Sparky",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "SP_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SP_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "on"

project "Sparky-Launcher"
	location "Sparky-Launcher"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"Sparky/src",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.spdlog}",
	}

	links
	{
		"Sparky",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "SP_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SP_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "on"
group ""


group "Runtime"
project "Sparky-Runtime"
	location "Sparky-Runtime"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"Sparky/src",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.spdlog}",
	}

	links
	{
		"Sparky",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "SP_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SP_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "on"
group ""


group "Misc"
project "Testbed"
	location "Testbed"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"Sparky/src",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.spdlog}",
	}

	links
	{
		"Sparky",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "SP_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SP_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "on"
group ""


workspace "Sandbox"
	architecture "x64"
	startproject "Sandbox"
	location "Sparky-Editor/SandboxProject/Assets/Scripts"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Sandbox"
	location "Sparky-Editor/SandboxProject/Assets/Scripts"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("Sparky-Editor/SandboxProject/Assets/Scripts/Binaries")
	objdir ("Sparky-Editor/SandboxProject/Assets/Scripts/Intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs",
	}

	links
	{
		"Sparky-ScriptCore",
	}

	filter "configurations:Debug"
		optimize "off"
		symbols "default"

	filter "configurations:Release"
		optimize "on"
		symbols "default"

	filter "configurations:Dist"
		optimize "full"
		symbols "off"