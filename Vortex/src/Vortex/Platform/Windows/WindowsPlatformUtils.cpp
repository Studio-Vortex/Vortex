#include "vxpch.h"
#include "Vortex/Utils/PlatformUtils.h"
#include "Vortex/Utils/FileSystem.h"

#include "Vortex/Core/Application.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <commdlg.h>
#include <shellapi.h>

namespace Vortex {

	Buffer FileSystem::ReadBinary(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return Buffer();
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();

		if (size == 0)
		{
			// File is empty
			return Buffer();
		}

		Buffer result(size);
		stream.read(result.As<char>(), result.Size);
		stream.close();

		return result;
	}

	std::string FileSystem::ReadText(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return "";
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint64_t size = end - stream.tellg();

		if (size == 0)
		{
			// File is empty
			return "";
		}

		std::string result;
		stream.read(result.data(), size);
		stream.close();

		return result;
	}

	bool FileSystem::Exists(const std::filesystem::path& filepath)
	{
		return std::filesystem::exists(filepath);
	}

	bool FileSystem::CreateDirectory(const std::filesystem::path& directory)
	{
		return std::filesystem::create_directory(directory);
	}

	bool FileSystem::CreateDirectories(const std::filesystem::path& directories)
	{
		return std::filesystem::create_directories(directories);
	}

	bool FileSystem::Remove(const std::filesystem::path& filepath)
	{
		return std::filesystem::remove(filepath);
	}

	void FileSystem::Copy(const std::filesystem::path& from, const std::filesystem::path& to)
	{
		std::filesystem::copy(from, to);
	}

	bool FileSystem::CopyFile(const std::filesystem::path& from, const std::filesystem::path& to)
	{
		return std::filesystem::copy_file(from, to);
	}

	std::filesystem::path FileSystem::GetParentDirectory(const std::filesystem::path& filepath)
	{
		if (filepath.has_parent_path())
			return filepath.parent_path();

		return "";
	}

	std::filesystem::path FileSystem::Absolute(const std::filesystem::path& filepath)
	{
		return std::filesystem::absolute(filepath);
	}

	std::filesystem::path FileSystem::Relative(const std::filesystem::path& filepath)
	{
		return std::filesystem::relative(filepath);
	}

	std::filesystem::path FileSystem::Relative(const std::filesystem::path& path, const std::filesystem::path& base)
	{
		return std::filesystem::relative(path, base);
	}

	std::filesystem::path FileSystem::GetCurrentPath()
	{
		return std::filesystem::current_path();
	}

	void FileSystem::SetCurrentPath(const std::filesystem::path& filepath)
	{
		std::filesystem::current_path(filepath);
	}

	void FileSystem::LaunchApplication(const char* binaryPath, const char* args)
	{
		ShellExecuteA(NULL, "open", binaryPath, args, NULL, SWP_SHOWWINDOW);
	}

	std::string FileDialogue::OpenFileDialog(const char* fileFilter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindowHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = fileFilter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}
	
	std::string FileDialogue::SaveFileDialog(const char* fileFilter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindowHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = fileFilter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}

	void FileDialogue::OpenInFileExplorer(const char* directoryName)
	{
		ShellExecuteA(NULL, "open", directoryName, NULL, NULL, SW_SHOWDEFAULT);
	}

	void Random::Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	float Random::Float()
	{
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}

	static float s_DeltaTime = 0.0f;

	float Time::GetTime()
	{
		return glfwGetTime();
	}

	float Time::GetDeltaTime()
	{
		return s_DeltaTime;
	}

	void Time::SetDeltaTime(float delta)
	{
		s_DeltaTime = delta;
	}

}
