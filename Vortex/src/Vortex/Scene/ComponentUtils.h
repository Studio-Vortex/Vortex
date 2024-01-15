#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Scene/Actor.h"
#include "Vortex/Scene/Components.h"

#include <entt/entt.hpp>

namespace Vortex {

	class VORTEX_API ComponentUtils
	{
	public:
		template <typename... TComponent>
		static void CopyComponent(entt::registry& dst, const entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
			([&]()
				{
					auto view = src.view<TComponent>();

					for (auto srcActor : view)
					{
						entt::entity dstActor = enttMap.at(src.get<IDComponent>(srcActor).ID);

						const auto& srcComponent = src.get<TComponent>(srcActor);
						dst.emplace_or_replace<TComponent>(dstActor, srcComponent);
					}
				}(), ...);
		}

		template<typename... Component>
		static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
			CopyComponent<Component...>(dst, src, enttMap);
		}

		template<typename... TComponent>
		static void CopyComponentIfExists(Actor dst, Actor src)
		{
			([&]()
				{
					if (src.HasComponent<TComponent>())
					{
						dst.AddOrReplaceComponent<TComponent>(src.GetComponent<TComponent>());
					}
				}(), ...);
		}

		template<typename... TComponent>
		static void CopyComponentIfExists(ComponentGroup<TComponent...>, Actor dst, Actor src)
		{
			CopyComponentIfExists<TComponent...>(dst, src);
		}
	};

}
