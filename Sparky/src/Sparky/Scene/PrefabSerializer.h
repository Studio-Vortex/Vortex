#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Scene/Prefab.h"

#include <string>

namespace Sparky {

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
