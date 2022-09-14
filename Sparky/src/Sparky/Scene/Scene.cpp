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

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdateRuntime(TimeStep delta)
	{
		// Update Scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				// TODO: Move to Scene::OnScenePlay
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };
					nsc.Instance->OnCreate();
				}

				nsc.Instance->OnUpdate(delta);
			});
		}

		// Render 2D
		Camera* mainCamera = nullptr;
		Math::mat4 cameraTransform;

		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();

			for (auto entity : view)
			{
				auto [transformComponent, cameraComponent] = view.get<TransformComponent, CameraComponent>(entity);

				if (cameraComponent.Primary)
				{
					mainCamera = &cameraComponent.Camera;
					cameraTransform = transformComponent.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);

			for (auto entity : group)
			{
				auto [transformComponent, spriteComponent] = group.get<TransformComponent, SpriteComponent>(entity);

				Renderer2D::DrawQuad(transformComponent.GetTransform(), spriteComponent.SpriteColor);
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateEditor(TimeStep delta, EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);

		for (auto entity : group)
		{
			auto [transformComponent, spriteComponent] = group.get<TransformComponent, SpriteComponent>(entity);

			Renderer2D::DrawQuad(transformComponent.GetTransform(), spriteComponent.SpriteColor);
		}

		Renderer2D::EndScene();
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();

		for (auto& entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);

			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

    Entity Scene::GetPrimaryCameraEntity()
    {
		auto view = m_Registry.view<CameraComponent>();

		for (auto& entity : view)
		{
			auto& cc = view.get<CameraComponent>(entity);
			if (cc.Primary)
				return { entity, this };
		}

		return {};
    }

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		SP_CORE_ASSERT(false, "Should not be calling base template function!");
	}

	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}
	
	template <>
	void Scene::OnComponentAdded<SpriteComponent>(Entity entity, SpriteComponent& component)
	{
	}
	
	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
	
	template <>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

}