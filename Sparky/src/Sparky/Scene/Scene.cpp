#include "sppch.h"
#include "Scene.h"

#include "Sparky/Scene/Components.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Renderer/Renderer2D.h"
#include "Sparky/Core/Math.h"

namespace Sparky {

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::OnUpdate(TimeStep delta)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<Sprite2DComponent>);

		for (auto entity : group)
		{
			const auto& [transform, sprite] = m_Registry.get<TransformComponent, Sprite2DComponent>(entity);

			Renderer2D::DrawQuad(transform, sprite.SpriteColor);
		}
	}

}