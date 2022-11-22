#pragma once

#include <Sparky.h>

namespace Sparky {

	class LauncherLayer : public Layer
	{
	public:
		LauncherLayer();
		~LauncherLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(TimeStep delta) override;
		void OnGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		void DisplayCreateProjectPopup();
		void LaunchEditor();

	private:
		SharedRef<Framebuffer> m_Framebuffer;
		SharedRef<Texture2D> m_SparkyLogoIcon;
		Math::vec2 m_ViewportSize = Math::vec2();
		std::filesystem::path m_ProjectPath = std::filesystem::path();

		bool m_CreateNewProject = false;
	};

}
