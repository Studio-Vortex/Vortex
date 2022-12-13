#pragma once

#include <Vortex.h>

namespace Vortex {

	struct LauncherProperties
	{
		std::string EditorPath;
	};

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
		LauncherProperties m_Properties;
		SharedRef<Framebuffer> m_Framebuffer;
		SharedRef<Texture2D> m_VortexLogoIcon;
		Math::vec2 m_ViewportSize = Math::vec2();
		std::filesystem::path m_ProjectPath = std::filesystem::path();

		bool m_CreatingNewProject = false;
	};

}
