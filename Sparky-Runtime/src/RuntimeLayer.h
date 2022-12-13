#pragma once

#include <Sparky.h>

namespace Sparky {

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
		void OnRuntimeScenePaused();
		void OnRuntimeSceneResumed();

		bool OpenProject(const std::filesystem::path& filepath);
		bool OpenScene(const std::filesystem::path& filepath);

	private:
		SharedRef<Framebuffer> m_Framebuffer;
		SharedRef<Scene> m_RuntimeScene = nullptr;
		Math::vec2 m_ViewportSize = Math::vec2();
		Entity m_HoveredEntity;

		std::vector<SharedRef<AudioSource>> m_AudioSourcesToResume = std::vector<SharedRef<AudioSource>>();
	};

}
