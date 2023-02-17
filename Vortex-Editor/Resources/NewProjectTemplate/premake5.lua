local VortexDir = os.getenv("VORTEX_DIR")

workspace "$PROJECT_NAME$"
	architecture "x64"
	startproject "$PROJECT_NAME$"
	location "."

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

project "$PROJECT_NAME$"
	location "."
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("./Binaries")
	objdir ("./Intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs",
	}

	links
	{
		("%{VortexDir}" .. "/Vortex-Editor/Resources/Scripts/Vortex-ScriptCore.dll"),
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