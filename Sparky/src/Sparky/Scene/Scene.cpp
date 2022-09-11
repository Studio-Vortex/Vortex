#include "sppch.h"
#include "Scene.h"

#include "Sparky/Core/Math.h"

namespace Sparky {

	Scene::Scene()
	{
		struct MeshComponent
		{

		};

		struct TransformComponent
		{
			Math::mat4 Transform;

			TransformComponent() = default;
			TransformComponent(const TransformComponent&) = default;
			TransformComponent(const Math::mat4& transform)
				: Transform(transform) {}

			operator Math::mat4& () { return Transform; }
			operator const Math::mat4& () const { return Transform; }
		};

		entt::entity entity = m_Registry.create();
		m_Registry.emplace<TransformComponent>(entity, Math::Identity());
		TransformComponent transform = m_Registry.get<TransformComponent>(entity);
		bool has_component = m_Registry.any_of<TransformComponent>(entity);

		auto view = m_Registry.view<TransformComponent>();

		for (auto& gameObject : view)
		{
			auto& transform = view.get<TransformComponent>(gameObject);
		}
		
		auto group = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);

		for (auto& gameObject : group)
		{
			auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(gameObject);
		}

	}

	Scene::~Scene()
	{
	}

}