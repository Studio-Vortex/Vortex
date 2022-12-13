#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Scene/Prefab.h"

#include <string>

namespace Vortex {

	struct PrefabComponent;

	class PrefabSerializer
	{
	public:
		PrefabSerializer(const SharedRef<Prefab>& prefab);

		bool Serialize(const std::filesystem::path& path);
		bool Deserialize(const std::filesystem::path& path, PrefabComponent& prefabComponent);

	private:
		SharedRef<Prefab> m_Prefab = nullptr;
	};

}
