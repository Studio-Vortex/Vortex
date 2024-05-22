#pragma once

#include "Vortex/Asset/Asset.h"
#include "Vortex/ReferenceCounting/SharedRef.h"
#include "Vortex/Scene/Actor.h"

namespace Vortex {

	class Scene;

	class VORTEX_API Prefab : public Asset
	{
	public:
		Prefab();
		~Prefab() override = default;

		void Create(Actor actor, bool serialize = true);

		ASSET_CLASS_TYPE(PrefabAsset)

	private:
		Actor CreatePrefabFromActor(Actor actor);

	private:
		SharedReference<Scene> m_Scene = nullptr;
		Actor m_Actor;

	private:
		friend class Scene;
		friend class ScriptEngine;
		friend class PrefabAssetSerializer;
	};

}
