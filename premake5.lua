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
IncludeDir["filewatch"] = "Sparky/vendor/filewatch"
IncludeDir["GLFW"] = "Sparky/vendor/GLFW/include"
IncludeDir["Glad"] = "Sparky/vendor/Glad/include"
IncludeDir["glm"] = "Sparky/vendor/glm"
IncludeDir["ImGui"] = "Sparky/vendor/imgui"
IncludeDir["ImGuizmo"] = "Sparky/vendor/ImGuizmo"
IncludeDir["ImGuiColorTextEdit"] = "Sparky/vendor/ImGuiColorTextEdit"
IncludeDir["miniaudio"] = "Sparky/vendor/miniaudio"
IncludeDir["mono"] = "%{wks.location}/Sparky/vendor/mono/include"
IncludeDir["PhysX"] = "Sparky/vendor/PhysX/include"
IncludeDir["qu3e"] = "Sparky/vendor/qu3e/src"
IncludeDir["spdlog"] = "Sparky/vendor/spdlog/include"
IncludeDir["stb_image"] = "Sparky/vendor/stb_image"
IncludeDir["tinygltf"] = "Sparky/vendor/tinygltf"
IncludeDir["tinyobjloader"] = "Sparky/vendor/tinyobjloader"
IncludeDir["yaml_cpp"] = "Sparky/vendor/yaml-cpp/include"

LibraryDir = {}
LibraryDir["Mono"] = "%{wks.location}/Sparky/vendor/mono/lib/%{cfg.buildcfg}"
LibraryDir["PhysX"] = "%{wks.location}/Sparky/vendor/PhysX/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.Mono}/mono-2.0-sgen.lib" -- dll binary must be included in build
--Library["mono"] = "%{LibraryDir.Mono}/libmono-static-sgen.lib"

Library["PhysX"] = "%{LibraryDir.PhysX}/PhysX_static_64.lib"
Library["PhysXCharacterKinematic"] = "%{LibraryDir.PhysX}/PhysXCharacterKinematic_static_64.lib"
Library["PhysXCommon"] = "%{LibraryDir.PhysX}/PhysXCommon_static_64.lib"
Library["PhysXCooking"] = "%{LibraryDir.PhysX}/PhysXCooking_static_64.lib"
Library["PhysXExtensions"] = "%{LibraryDir.PhysX}/PhysXExtensions_static_64.lib"
Library["PhysXFoundation"] = "%{LibraryDir.PhysX}/PhysXFoundation_static_64.lib"
Library["PhysXPvd"] = "%{LibraryDir.PhysX}/PhysXPvdSDK_static_64.lib"

group "External Dependencies"
	include "Sparky/vendor/Box2D"
	include "Sparky/vendor/GLFW"
	include "Sparky/vendor/Glad"
	include "Sparky/vendor/imgui"
	include "Sparky/vendor/qu3e"
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
		"%{prj.name}/vendor/tinygltf/**.h",
		"%{prj.name}/vendor/tinygltf/**.hpp",
		"%{prj.name}/vendor/tinygltf/**.cpp",
		"%{prj.name}/vendor/tinyobjloader/tiny_obj_loader.h",

		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp",

		"%{prj.name}/vendor/ImGuiColorTextEdit/TextEditor.h",
		"%{prj.name}/vendor/ImGuiColorTextEdit/TextEditor.cpp",
	}

	defines
	{
		"PX_PHYSX_STATIC_LIB", "_CRT_SECURE_NO_WARNINGS",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.ImGuiColorTextEdit}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.qu3e}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.tinyobjloader}",
		"%{IncludeDir.yaml_cpp}",
	}

	links
	{
		"Box2D",
		"Glad",
		"GLFW",
		"ImGui",
		"%{Library.mono}",
		"%{Library.PhysX}",
		"%{Library.PhysXCharacterKinematic}",
		"%{Library.PhysXCommon}",
		"%{Library.PhysXCooking}",
		"%{Library.PhysXExtensions}",
		"%{Library.PhysXFoundation}",
		"%{Library.PhysXPvd}",
		"opengl32.lib",
		"qu3e",
		"yaml-cpp",
	}

	filter "files:Sparky/vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "files:Sparky/vendor/ImGuiColorTextEdit/**.cpp"
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
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

	filter "configurations:Dist"
		defines "SP_DIST"
		runtime "Release"
		optimize "on"
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}


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
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.ImGuiColorTextEdit}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.tinyobjloader}",
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
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.tinyobjloader}",
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