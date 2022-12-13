#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Project/Project.h"

#include <string>

namespace Vortex {

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
