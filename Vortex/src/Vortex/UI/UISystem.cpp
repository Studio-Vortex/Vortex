#include "vxpch.h"
#include "UISystem.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Scene/Scene.h"

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
		moduleProps.RequiredModules = {};
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
