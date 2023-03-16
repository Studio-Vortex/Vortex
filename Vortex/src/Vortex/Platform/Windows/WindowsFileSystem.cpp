#include "vxpch.h"
#include "Vortex/Utils/FileSystem.h"

#include <ShlObj.h>
#include <shellapi.h>

#include <fstream>

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

	bool FileSystem::CreateDirectoryV(const std::filesystem::path& directory)
	{
		return std::filesystem::create_directory(directory);
	}

	bool FileSystem::CreateDirectoriesV(const std::filesystem::path& directories)
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

	bool FileSystem::CopyFileV(const std::filesystem::path& from, const std::filesystem::path& to)
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

    std::string FileSystem::GetFileExtension(const std::filesystem::path& filepath)
    {
		if (filepath.has_extension())
		{
			return filepath.extension().string();
		}

		return "";
    }

	std::string FileSystem::RemoveFileExtension(const std::filesystem::path& filepath)
	{
		std::string filename = filepath.stem().string();
		return filename;
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

}
