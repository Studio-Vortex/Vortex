#include "vxpch.h"
#include "UISystem.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Renderer/Renderer2D.h"

namespace Vortex {

	struct UISystemInternalData
	{
		using UIButtonData = std::unordered_map<UUID, UUID>;
		UIButtonData ButtonData;

		SubModule Module;
	};

	static UISystemInternalData s_Data;

	UISystem::UISystem()
		: ISystem("UI System")
	{
	}

	void UISystem::Init()
	{
		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "UI";
		moduleProps.APIVersion = Version(1, 0, 0);
		moduleProps.RequiredModules = { "Renderer" };
		s_Data.Module.Init(moduleProps);

		Application::Get().AddModule(s_Data.Module);
	}

	void UISystem::Shutdown()
	{
		s_Data.ButtonData.clear();

		Application::Get().RemoveModule(s_Data.Module);
		s_Data.Module.Shutdown();
	}

	void UISystem::OnContextSceneCreated(Scene* context)
	{
	}

	void UISystem::OnContextSceneDestroyed(Scene* context)
	{
	}

	void UISystem::OnRuntimeStart(Scene* context)
	{
	}

	void UISystem::OnUpdateRuntime(Scene* context)
	{
		auto buttonView = context->GetAllActorsWith<ButtonComponent>();

		Actor primaryCamera = context->GetPrimaryCameraActor();
		if (!primaryCamera || !context->IsRunning())
			return;

		const CameraComponent& cc = primaryCamera.GetComponent<CameraComponent>();
		const SceneCamera& camera = cc.Camera;

		const Math::mat4 transform = context->GetWorldSpaceTransformMatrix(primaryCamera);
		const Math::mat4 cameraView = Math::Inverse(transform);

		Renderer2D::BeginScene(camera, cameraView);

		for (const auto e : buttonView)
		{
			Actor actor{ e, context };

			const ButtonComponent& buttonComponent = actor.GetComponent<ButtonComponent>();

			if (!buttonComponent.Visible)
				continue;

			TransformComponent transform = context->GetWorldSpaceTransform(actor);

			Renderer2D::DrawQuadBillboard(
				cameraView,
				transform.Translation,
				Math::vec2(transform.Scale),
				buttonComponent.BackgroundColor,
				(int)(entt::entity)e
			);

			const AssetHandle fontHandle = buttonComponent.Font.FontAsset;
			SharedReference<Font> font = nullptr;

			if (AssetManager::IsHandleValid(fontHandle))
			{
				font = AssetManager::GetAsset<Font>(fontHandle);
			}
			else
			{
				font = Font::GetDefaultFont();
			}

			const Math::vec3 backward = transform.CalculateBackward();

			// NOTE: we need to be adding in world units here otherwise the text won't be visible
			transform.Translation += Math::vec3(buttonComponent.Font.Offset, 0.0f) + (backward * 0.01f);
			transform.Scale = Math::vec3(buttonComponent.Font.Scale * Math::vec2(transform.Scale), transform.Scale.z);

			Renderer2D::DrawString(
				buttonComponent.Font.TextString,
				font,
				transform.GetTransform(),
				buttonComponent.Font.MaxWidth,
				buttonComponent.Font.Color,
				buttonComponent.Font.BackgroundColor,
				buttonComponent.Font.LineSpacing,
				buttonComponent.Font.Kerning,
				(int)(entt::entity)e
			);
		}

		Renderer2D::EndScene();
	}

	void UISystem::OnRuntimeScenePaused(Scene* context)
	{
	}

	void UISystem::OnRuntimeSceneResumed(Scene* context)
	{
	}

	void UISystem::OnRuntimeStop(Scene* context)
	{
	}

	void UISystem::OnGuiRender()
	{
	}

}
