#pragma once

#include <Vortex.h>

namespace Vortex {

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer();
		~RuntimeLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(TimeStep delta) override;
		void OnGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		void OnRuntimeScenePlay();
		void OnRuntimeSceneStop();

		bool OnWindowCloseEvent(WindowCloseEvent& e);

		bool OpenProject(const std::filesystem::path& filepath);
		void CloseProject();
		bool OpenScene(const AssetMetadata& sceneMetadata);

		void SetSceneBuildIndexFromMetadata(const AssetMetadata& sceneMetadata);

		void QueueSceneTransition();

	private:
		SharedReference<Framebuffer> m_Framebuffer = nullptr;
		SharedReference<Scene> m_RuntimeScene = nullptr;
		Math::vec2 m_ViewportSize = Math::vec2();
		Entity m_HoveredEntity;
	};

}
