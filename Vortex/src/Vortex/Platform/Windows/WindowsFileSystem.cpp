#include "vxpch.h"
#include "Vortex/Utils/FileSystem.h"

#include <fstream>

namespace Vortex {

	Buffer FileSystem::ReadBinary(const Fs::Path& filepath)
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

	std::string FileSystem::ReadText(const Fs::Path& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return "";
		}

		const std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		const std::streampos start = stream.tellg();
		const uint64_t fileSize = end - start;

		if (fileSize == 0)
		{
			// File is empty
			return "";
		}

		std::string result;
		result.resize(fileSize);
		stream.read(result.data(), fileSize);
		stream.close();

		return result;
	}

	bool FileSystem::Exists(const Fs::Path& filepath)
	{
		return std::filesystem::exists(filepath);
	}

    bool FileSystem::Equivalent(const Fs::Path& first, const Fs::Path& second)
    {
		return std::filesystem::equivalent(first, second);
    }

    bool FileSystem::IsDirectory(const Fs::Path& filepath)
    {
        return std::filesystem::is_directory(filepath);
    }

	bool FileSystem::CreateDirectoryV(const Fs::Path& directory)
	{
		return std::filesystem::create_directory(directory);
	}

	bool FileSystem::CreateDirectoriesV(const Fs::Path& directories)
	{
		return std::filesystem::create_directories(directories);
	}

	bool FileSystem::Remove(const Fs::Path& filepath)
	{
		return std::filesystem::remove(filepath);
	}

    bool FileSystem::CreateFileV(const Fs::Path& filepath)
    {
		std::ofstream fout(filepath);
		bool success = fout.is_open();
		fout.close();

		return success;
    }

	void FileSystem::Copy(const Fs::Path& from, const Fs::Path& to)
	{
		std::filesystem::copy(from, to);
	}

	bool FileSystem::CopyFileV(const Fs::Path& from, const Fs::Path& to)
	{
		return std::filesystem::copy_file(from, to);
	}

    void FileSystem::Rename(const Fs::Path& old, const Fs::Path& _new)
    {
		std::filesystem::rename(old, _new);
    }

    void FileSystem::RecursiveDirectoryCopy(const Fs::Path& from, const Fs::Path& to)
    {
		std::filesystem::copy(from, to, std::filesystem::copy_options::recursive);
    }

	Fs::Path FileSystem::GetParentDirectory(const Fs::Path& filepath)
	{
		if (filepath.has_parent_path())
			return filepath.parent_path();

		return "";
	}

	Fs::Path FileSystem::Absolute(const Fs::Path& filepath)
	{
		return std::filesystem::absolute(filepath);
	}

	Fs::Path FileSystem::Relative(const Fs::Path& filepath)
	{
		return std::filesystem::relative(filepath);
	}

	Fs::Path FileSystem::Relative(const Fs::Path& path, const Fs::Path& base)
	{
		return std::filesystem::relative(path, base);
	}

	bool FileSystem::HasFileExtension(const Fs::Path& filepath)
	{
		return filepath.has_extension();
	}

	std::string FileSystem::GetFileExtension(const Fs::Path& filepath)
    {
		if (filepath.has_extension())
		{
			return filepath.extension().string();
		}

		return "";
    }

	std::string FileSystem::RemoveFileExtension(const Fs::Path& filepath)
	{
		return filepath.stem().string();
	}

    void FileSystem::ReplaceExtension(Fs::Path& filepath, std::string_view extension)
    {
		filepath.replace_extension(extension);
    }

	Fs::Path FileSystem::GetWorkingDirectory()
	{
		return std::filesystem::current_path();
	}

	void FileSystem::SetWorkingDirectory(const Fs::Path& filepath)
	{
		std::filesystem::current_path(filepath);
	}

}
