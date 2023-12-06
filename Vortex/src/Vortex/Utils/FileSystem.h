#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Buffer.h"

#include <filesystem>

namespace Vortex {

	// FileSystem
	namespace Fs {

		using Path = std::filesystem::path;

	}

	class VORTEX_API FileSystem
	{
	public:
		static Buffer ReadBinary(const Fs::Path& filepath);
		static std::string ReadText(const Fs::Path& filepath);

		static bool Exists(const Fs::Path& filepath);
		static bool Equivalent(const Fs::Path& first, const Fs::Path& second);

		static bool IsDirectory(const Fs::Path& filepath);

		static bool CreateDirectoryV(const Fs::Path& directory);
		static bool CreateDirectoriesV(const Fs::Path& directories);
		static bool Remove(const Fs::Path& filepath);

		static bool CreateFileV(const Fs::Path& filepath);

		static void Copy(const Fs::Path& from, const Fs::Path& to);
		static bool CopyFileV(const Fs::Path& from, const Fs::Path& to);

		static void Rename(const Fs::Path& old, const Fs::Path& _new);

		static void RecursiveDirectoryCopy(const Fs::Path& from, const Fs::Path& to);

		static Fs::Path GetParentDirectory(const Fs::Path& filepath);
		static Fs::Path Absolute(const Fs::Path& filepath);
		static Fs::Path Relative(const Fs::Path& filepath);
		static Fs::Path Relative(const Fs::Path& path, const Fs::Path& base);

		static bool HasFileExtension(const Fs::Path& filepath);
		static std::string GetFileExtension(const Fs::Path& filepath);
		static std::string RemoveFileExtension(const Fs::Path& filepath);
		static void ReplaceExtension(Fs::Path& filepath, std::string_view extension);

		static Fs::Path GetWorkingDirectory();
		static void SetWorkingDirectory(const Fs::Path& filepath);
	};

}
