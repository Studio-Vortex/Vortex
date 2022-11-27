#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Project/Project.h"

#include <string>

namespace Sparky {

	class ProjectSerializer
	{
	public:
		ProjectSerializer(const SharedRef<Project>& project);

		bool Serialize(const std::filesystem::path& path);
		bool Deserialize(const std::filesystem::path& path);

	private:
		SharedRef<Project> m_Project = nullptr;
	};

}
