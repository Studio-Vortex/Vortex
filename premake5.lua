workspace "Vortex"
	architecture "x64"
	startproject "Vortex-Editor"

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
IncludeDir["Assimp"] = "Vortex/vendor/assimp/include"
IncludeDir["Box2D"] = "Vortex/vendor/Box2D/include"
IncludeDir["entt"] = "Vortex/vendor/entt/include"
IncludeDir["filewatch"] = "Vortex/vendor/filewatch"
IncludeDir["GLFW"] = "Vortex/vendor/GLFW/include"
IncludeDir["Glad"] = "Vortex/vendor/Glad/include"
IncludeDir["glm"] = "Vortex/vendor/glm"
IncludeDir["ImGui"] = "Vortex/vendor/imgui"
IncludeDir["ImGuizmo"] = "Vortex/vendor/ImGuizmo"
IncludeDir["ImGuiColorTextEdit"] = "Vortex/vendor/ImGuiColorTextEdit"
IncludeDir["miniaudio"] = "Vortex/vendor/miniaudio"
IncludeDir["mono"] = "%{wks.location}/Vortex/vendor/mono/include"
IncludeDir["msdf_atlas_gen"] = "Vortex/vendor/msdf-atlas-gen/msdf-atlas-gen"
IncludeDir["msdfgen"] = "Vortex/vendor/msdf-atlas-gen/msdfgen"
IncludeDir["PhysX"] = "Vortex/vendor/PhysX/include"
IncludeDir["spdlog"] = "Vortex/vendor/spdlog/include"
IncludeDir["stb_image"] = "Vortex/vendor/stb_image"
IncludeDir["yaml_cpp"] = "Vortex/vendor/yaml-cpp/include"

LibraryDir = {}
LibraryDir["Mono"] = "%{wks.location}/Vortex/vendor/mono/lib/%{cfg.buildcfg}"
LibraryDir["PhysX"] = "%{wks.location}/Vortex/vendor/PhysX/lib/%{cfg.buildcfg}"

Library = {}
Library["Assimp_Debug"] = "%{wks.location}/Vortex/vendor/assimp/bin/Debug/assimp-vc142-mtd.lib"
Library["Assimp_Release"] = "%{wks.location}/Vortex/vendor/assimp/bin/Release/assimp-vc142-mt.lib"
Library["mono"] = "%{LibraryDir.Mono}/mono-2.0-sgen.lib" -- dll binary must be included in build
Library["PhysX"] = "%{LibraryDir.PhysX}/PhysX_static_64.lib"
Library["PhysXCharacterKinematic"] = "%{LibraryDir.PhysX}/PhysXCharacterKinematic_static_64.lib"
Library["PhysXCommon"] = "%{LibraryDir.PhysX}/PhysXCommon_static_64.lib"
Library["PhysXCooking"] = "%{LibraryDir.PhysX}/PhysXCooking_static_64.lib"
Library["PhysXExtensions"] = "%{LibraryDir.PhysX}/PhysXExtensions_static_64.lib"
Library["PhysXFoundation"] = "%{LibraryDir.PhysX}/PhysXFoundation_static_64.lib"
Library["PhysXPvd"] = "%{LibraryDir.PhysX}/PhysXPvdSDK_static_64.lib"

Binaries = {}
Binaries["Assimp_Debug"] = "%{wks.location}/Vortex/vendor/assimp/bin/Debug/assimp-vc142-mtd.dll"
Binaries["Assimp_Release"] = "%{wks.location}/Vortex/vendor/assimp/bin/Release/assimp-vc142-mt.dll"

group "External Dependencies"
	include "Vortex/vendor/Box2D"
	include "Vortex/vendor/GLFW"
	include "Vortex/vendor/Glad"
	include "Vortex/vendor/imgui"
	include "Vortex/vendor/yaml-cpp"

	group "External Dependencies/msdf"
		include "Vortex/vendor/msdf-atlas-gen"
	group ""
group ""


group "Core"
project "Vortex"
	location "Vortex"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "vxpch.h"
	pchsource "Vortex/src/vxpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",

		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",

		"%{prj.name}/vendor/miniaudio/miniaudio/miniaudio.h",

		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp",

		"%{prj.name}/vendor/ImGuiColorTextEdit/TextEditor.h",
		"%{prj.name}/vendor/ImGuiColorTextEdit/TextEditor.cpp",
	}

	defines
	{
		"PX_PHYSX_STATIC_LIB",
		"_CRT_SECURE_NO_WARNINGS",
	}

	includedirs
	{
		"%{prj.name}/src",

		"%{IncludeDir.Assimp}",
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
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.PhysX}",
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
		"msdf-atlas-gen",
		"%{Library.PhysX}",
		"%{Library.PhysXCharacterKinematic}",
		"%{Library.PhysXCommon}",
		"%{Library.PhysXCooking}",
		"%{Library.PhysXExtensions}",
		"%{Library.PhysXFoundation}",
		"%{Library.PhysXPvd}",
		"opengl32.lib",
		"d3d11.lib",
		"Ws2_32.lib",
		"yaml-cpp",
	}

	filter "files:Vortex/vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "files:Vortex/vendor/ImGuiColorTextEdit/**.cpp"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"VX_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
		}

	filter "configurations:Debug"
		defines "VX_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "VX_RELEASE"
		runtime "Release"
		optimize "on"
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

	filter "configurations:Dist"
		defines "VX_DIST"
		runtime "Release"
		optimize "on"
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}


project "Vortex-ScriptCore"
	location "Vortex-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("Vortex-Editor/Resources/Scripts")
	objdir ("Vortex-Editor/Resources/Scripts/Intermediates")

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
project "Vortex-Editor"
	location "Vortex-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"Vortex/src",

		"%{IncludeDir.Assimp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.filewatch}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.ImGuiColorTextEdit}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/PhysX",
		"%{IncludeDir.spdlog}",
	}
	
	defines
	{
		"PX_PHYSX_STATIC_LIB",
	}

	links
	{
		"Vortex",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "VX_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.Assimp_Debug}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Debug}" "%{cfg.targetdir}"',
		}

	filter "configurations:Release"
		defines "VX_RELEASE"
		runtime "Release"
		optimize "on"
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.Assimp_Release}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}

	filter "configurations:Dist"
		defines "VX_DIST"
		runtime "Release"
		optimize "on"
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.Assimp_Release}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}

project "Vortex-Launcher"
	location "Vortex-Launcher"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"Vortex/src",

		"%{IncludeDir.Assimp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/PhysX",
		"%{IncludeDir.spdlog}",
	}

	defines
	{
		"PX_PHYSX_STATIC_LIB",
	}

	links
	{
		"Vortex",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "VX_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.Assimp_Debug}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Debug}" "%{cfg.targetdir}"',
		}

	filter "configurations:Release"
		defines "VX_RELEASE"
		runtime "Release"
		optimize "on"
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.Assimp_Release}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}

	filter "configurations:Dist"
		defines "VX_DIST"
		runtime "Release"
		optimize "on"
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.Assimp_Release}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}
group ""


group "Runtime"
project "Vortex-Runtime"
	location "Vortex-Runtime"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"Vortex/src",

		"%{IncludeDir.Assimp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/PhysX",
		"%{IncludeDir.spdlog}",
	}

	defines
	{
		"PX_PHYSX_STATIC_LIB",
	}

	links
	{
		"Vortex",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "VX_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.Assimp_Debug}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Debug}" "%{cfg.targetdir}"',
		}

	filter "configurations:Release"
		defines "VX_RELEASE"
		runtime "Release"
		optimize "on"

		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.Assimp_Release}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}

	filter "configurations:Dist"
		defines "VX_DIST"
		runtime "Release"
		optimize "on"

		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.Assimp_Release}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}
group ""


group "Misc"
project "Testbed"
	location "Testbed"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	includedirs
	{
		"Vortex/src",

		"%{IncludeDir.Assimp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX}/PhysX",
		"%{IncludeDir.spdlog}",
	}

	defines
	{
		"PX_PHYSX_STATIC_LIB",
	}

	links
	{
		"Vortex",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "VX_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.Assimp_Debug}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Debug}" "%{cfg.targetdir}"',
		}

	filter "configurations:Release"
		defines "VX_RELEASE"
		runtime "Release"
		optimize "on"
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.Assimp_Release}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}

	filter "configurations:Dist"
		defines "VX_DIST"
		runtime "Release"
		optimize "on"
		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.Assimp_Release}",
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp_Release}" "%{cfg.targetdir}"',
		}
group ""


workspace "Sandbox"
	architecture "x64"
	startproject "Sandbox"
	location "Vortex-Editor/SandboxProject/Assets/Scripts"

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
	location "Vortex-Editor/SandboxProject/Assets/Scripts"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("Vortex-Editor/SandboxProject/Assets/Scripts/Binaries")
	objdir ("Vortex-Editor/SandboxProject/Assets/Scripts/Intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs",
	}

	links
	{
		"Vortex-ScriptCore",
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