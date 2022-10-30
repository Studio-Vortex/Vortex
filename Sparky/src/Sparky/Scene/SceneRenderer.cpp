#include "sppch.h"
#include "SceneRenderer.h"

#include "Sparky/Renderer/Renderer.h"
#include "Sparky/Renderer/Renderer2D.h"

namespace Sparky {

	template <typename TCamera>
	static void RenderScene(TCamera& camera, const Math::mat4& cameraTransform, entt::registry& sceneRegistry)
	{
		// Render 2D
		{
			if (typeid(TCamera).name() == typeid(EditorCamera).name())
				Renderer2D::BeginScene(reinterpret_cast<EditorCamera&>(camera));
			else
				Renderer2D::BeginScene(reinterpret_cast<Camera&>(camera), cameraTransform);

			// Render Sprites
			{
				auto view = sceneRegistry.view<TransformComponent, SpriteRendererComponent>();

				for (const auto entity : view)
				{
					auto [transformComponent, spriteComponent] = view.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transformComponent.GetTransform(), spriteComponent, (int)(entt::entity)entity);
				}
			}

			// Render Circles
			{
				auto group = sceneRegistry.group<TransformComponent>(entt::get<CircleRendererComponent>);

				for (const auto entity : group)
				{
					auto [transformComponent, circle] = group.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(transformComponent.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)(entt::entity)entity);
				}
			}

			Renderer2D::EndScene();
		}

		// Render 3D
		{
			if (typeid(TCamera).name() == typeid(EditorCamera).name())
				Renderer::BeginScene(reinterpret_cast<EditorCamera&>(camera));
			else
				Renderer::BeginScene(reinterpret_cast<Camera&>(camera), cameraTransform);

			// Render Lights
			{
				auto view = sceneRegistry.view<TransformComponent, LightComponent>();

				if (typeid(TCamera).name() == typeid(EditorCamera).name())
					Renderer2D::BeginScene(reinterpret_cast<EditorCamera&>(camera));
				else
					Renderer2D::BeginScene(reinterpret_cast<Camera&>(camera), cameraTransform);
				
				for (const auto entity : view)
				{
					auto [transformComponent, lightComponent] = view.get<TransformComponent, LightComponent>(entity);

					Renderer::RenderLight(transformComponent, lightComponent, (int)(entt::entity)entity);
				}

				Renderer2D::EndScene();
			}

			// Render Meshes
			{
				auto view = sceneRegistry.view<TransformComponent, MeshRendererComponent>();

				for (const auto entity : view)
				{
					auto [transformComponent, meshRendererComponent] = view.get<TransformComponent, MeshRendererComponent>(entity);

					if (meshRendererComponent.Mesh)
						Renderer::DrawModel(transformComponent, meshRendererComponent, (int)(entt::entity)entity);
				}
			}

			Renderer::EndScene();
		}
	}

	void SceneRenderer::RenderFromSceneCamera(SceneCamera* sceneCamera, const Math::mat4& cameraTransform, entt::registry& sceneRegistry)
	{
		RenderScene(*sceneCamera, cameraTransform, sceneRegistry);
	}

	void SceneRenderer::RenderFromEditorCamera(EditorCamera& editorCamera, entt::registry& sceneRegistry)
	{
		RenderScene(editorCamera, Math::Identity(), sceneRegistry);
	}

}
