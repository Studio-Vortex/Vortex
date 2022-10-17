#include "sppch.h"
#include "SceneRenderer.h"

#include "Sparky/Renderer/Renderer.h"
#include "Sparky/Renderer/Renderer2D.h"

namespace Sparky {

	void SceneRenderer::RenderFromSceneCamera(SceneCamera* sceneCamera, const Math::mat4& cameraTransform, entt::registry& sceneRegistry)
	{
		// Render 2D Primitives
		{
			Renderer2D::BeginScene(*sceneCamera, cameraTransform);

			/// Render Sprites
			{
				auto group = sceneRegistry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto entity : group)
				{
					auto [transformComponent, spriteComponent] = group.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transformComponent.GetTransform(), spriteComponent, (int)entity);
				}
			}

			/// Render Circles
			{
				auto view = sceneRegistry.view<TransformComponent, CircleRendererComponent>();
				for (auto entity : view)
				{
					auto [transformComponent, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(transformComponent.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
				}
			}

			Renderer2D::EndScene();
		}

		// Render 3D Primitives
		{
			Renderer::BeginScene(*sceneCamera, cameraTransform);

			/// Render Cubes
			{
				auto view = sceneRegistry.view<TransformComponent, MeshRendererComponent>();

				for (auto& entity : view)
				{
					auto [transformComponent, meshRendererComponent] = view.get<TransformComponent, MeshRendererComponent>(entity);

					if (meshRendererComponent.Type == MeshRendererComponent::MeshType::Cube)
						Renderer::DrawCube(transformComponent.GetTransform(), meshRendererComponent, (int)entity);
				}
			}

			Renderer::EndScene();
		}
	}

	void SceneRenderer::RenderFromEditorCamera(const EditorCamera& editorCamera, entt::registry& sceneRegistry)
	{
		// Render 2D Primitives
		{
			Renderer2D::BeginScene(editorCamera);

			/// Render Sprites
			{
				auto view = sceneRegistry.view<TransformComponent, SpriteRendererComponent>();

				for (auto entity : view)
				{
					auto [transformComponent, spriteComponent] = view.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transformComponent.GetTransform(), spriteComponent, (int)entity);
				}
			}

			/// Render Circles
			{
				auto group = sceneRegistry.group<TransformComponent>(entt::get<CircleRendererComponent>);

				for (auto entity : group)
				{
					auto [transformComponent, circle] = group.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(transformComponent.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
				}
			}

			Renderer2D::EndScene();
		}

		// Render 3D Primitives
		{
			Renderer::BeginScene(editorCamera);

			/// Render Cubes
			{
				auto view = sceneRegistry.view<TransformComponent, MeshRendererComponent>();

				for (auto& entity : view)
				{
					auto [transformComponent, meshRendererComponent] = view.get<TransformComponent, MeshRendererComponent>(entity);

					switch (meshRendererComponent.Type)
					{
						case MeshRendererComponent::MeshType::Cube:
						{
							Renderer::DrawCube(transformComponent.GetTransform(), meshRendererComponent, (int)entity);
							break;
						}
					}
				}
			}

			Renderer::EndScene();
		}
	}

}
