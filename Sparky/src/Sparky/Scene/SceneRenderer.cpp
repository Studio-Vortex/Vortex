#include "sppch.h"
#include "SceneRenderer.h"

#include "Sparky/Renderer/Renderer.h"
#include "Sparky/Renderer/Renderer2D.h"

namespace Sparky {

	void SceneRenderer::RenderFromSceneCamera(SceneCamera* sceneCamera, const Math::mat4& cameraTransform, entt::registry& sceneRegistry)
	{
		// Render 2D
		{
			Renderer2D::BeginScene(*sceneCamera, cameraTransform);

			// Render Sprites
			{
				auto group = sceneRegistry.group<TransformComponent>(entt::get<SpriteRendererComponent>);

				for (const auto entity : group)
				{
					auto [transformComponent, spriteComponent] = group.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transformComponent.GetTransform(), spriteComponent, (int)(entt::entity)entity);
				}
			}

			// Render Circles
			{
				auto view = sceneRegistry.view<TransformComponent, CircleRendererComponent>();

				for (const auto entity : view)
				{
					auto [transformComponent, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(transformComponent.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)(entt::entity)entity);
				}
			}

			Renderer2D::EndScene();
		}

		// Render 3D
		{
			Renderer::BeginScene(*sceneCamera, cameraTransform);

			// Render Meshes
			{
				auto view = sceneRegistry.view<TransformComponent, MeshRendererComponent>();

				for (const auto entity : view)
				{
					auto [transformComponent, meshRendererComponent] = view.get<TransformComponent, MeshRendererComponent>(entity);

					if (meshRendererComponent.Mesh)
						Renderer::DrawModel(*sceneCamera, transformComponent.GetTransform(), meshRendererComponent, (int)(entt::entity)entity);
					else
					{
						switch (meshRendererComponent.Type)
						{
							case MeshRendererComponent::MeshType::Cube:
							{
								Renderer::DrawCube(transformComponent.GetTransform(), meshRendererComponent, (int)(entt::entity)entity);
								break;
							}
							case MeshRendererComponent::MeshType::Sphere:
							{
								Renderer::DrawSphere(transformComponent.GetTransform(), meshRendererComponent, (int)(entt::entity)entity);
								break;
							}
						}
					}
				}
			}

			Renderer::EndScene();
		}
	}

	void SceneRenderer::RenderFromEditorCamera(const EditorCamera& editorCamera, entt::registry& sceneRegistry)
	{
		// Render 2D
		{
			Renderer2D::BeginScene(editorCamera);

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
			Renderer::BeginScene(editorCamera);

			// Render Meshes
			{
				auto view = sceneRegistry.view<TransformComponent, MeshRendererComponent>();

				for (const auto entity : view)
				{
					auto [transformComponent, meshRendererComponent] = view.get<TransformComponent, MeshRendererComponent>(entity);

					if (meshRendererComponent.Mesh)
						Renderer::DrawModel(editorCamera, transformComponent.GetTransform(), meshRendererComponent, (int)(entt::entity)entity);
					else
					{
						switch (meshRendererComponent.Type)
						{
							case MeshRendererComponent::MeshType::Cube:
							{
								Renderer::DrawCube(transformComponent.GetTransform(), meshRendererComponent, (int)(entt::entity)entity);
								break;
							}
							case MeshRendererComponent::MeshType::Sphere:
							{
								Renderer::DrawSphere(transformComponent.GetTransform(), meshRendererComponent, (int)(entt::entity)entity);
								break;
							}
						}
					}
				}
			}

			Renderer::EndScene();
		}
	}

}
