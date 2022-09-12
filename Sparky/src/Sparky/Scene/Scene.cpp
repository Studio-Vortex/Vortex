#include "sppch.h"
#include "Scene.h"

#include "Sparky/Scene/Components.h"
#include "Sparky/Renderer/Renderer2D.h"
#include "Sparky/Core/Math.h"

namespace Sparky {

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
	}

	entt::entity Scene::CreateEntity()
	{
		return m_Registry.create();
	}

	void Scene::OnUpdate(TimeStep delta)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<Sprite2DComponent>);

		for (auto entity : group)
		{
			const auto& [transform, sprite] = group.get<TransformComponent, Sprite2DComponent>(entity);

			Renderer2D::DrawQuad(transform, sprite.SpriteColor);
		}
	}

}