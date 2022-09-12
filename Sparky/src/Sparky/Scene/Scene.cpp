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
		// Render 2D
		Camera* mainCamera = nullptr;
		Math::mat4* cameraTransform = nullptr;

		{
			auto group = m_Registry.view<TransformComponent, CameraComponent>();

			for (auto& entity : group)
			{
				auto& transform = group.get<TransformComponent>(entity);
				auto& camera = group.get<CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = &transform.Transform;
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, *cameraTransform);

			auto view = m_Registry.view<TransformComponent, Sprite2DComponent>();

			for (auto entity : view)
			{
				const auto& [transform, sprite] = view.get<TransformComponent, Sprite2DComponent>(entity);

				Renderer2D::DrawQuad(transform, sprite.SpriteColor);
			}

			Renderer2D::EndScene();
		}
	}

}