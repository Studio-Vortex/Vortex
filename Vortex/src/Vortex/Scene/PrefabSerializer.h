#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Scene/Prefab.h"

#include <string>

namespace Vortex {

	struct PrefabComponent;

	class PrefabSerializer
	{
	public:
		PrefabSerializer() = default;
		PrefabSerializer(const SharedRef<Prefab>& prefab);

		bool Serialize(const std::string& filepath);
		bool Deserialize(const std::string& filepath, PrefabComponent& prefabComponent);

	private:
		SharedRef<Prefab> m_Prefab = nullptr;
	};

}
