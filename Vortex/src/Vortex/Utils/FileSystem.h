#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Buffer.h"

#include <filesystem>

namespace Vortex {

	class VORTEX_API FileSystem
	{
	public:
		static Buffer ReadBinary(const std::filesystem::path& filepath);
		static std::string ReadText(const std::filesystem::path& filepath);
		static bool Exists(const std::filesystem::path& filepath);
		static bool CreateDirectory(const std::filesystem::path& directory);
		static bool CreateDirectories(const std::filesystem::path& directories);
		static bool Remove(const std::filesystem::path& filepath);
		static void Copy(const std::filesystem::path& from, const std::filesystem::path& to);
		static bool CopyFile(const std::filesystem::path& from, const std::filesystem::path& to);
		static std::filesystem::path GetParentDirectory(const std::filesystem::path& filepath);
		static std::filesystem::path Absolute(const std::filesystem::path& filepath);
		static std::filesystem::path Relative(const std::filesystem::path& filepath);
		static std::filesystem::path Relative(const std::filesystem::path& path, const std::filesystem::path& base);
		static std::filesystem::path GetCurrentPath();
		static void SetCurrentPath(const std::filesystem::path& filepath);
		static void LaunchApplication(const char* binaryPath, const char* args);
	};

}
