#pragma once

#include <Vortex.h>

namespace Vortex {

#define VX_MAX_PROJECT_NAME_LENGTH 256
#define VX_MAX_PROJECT_DIR_LENGTH 256

	struct LauncherProperties
	{
		std::string EditorPath;
		std::string WorkingDirectory;
		inline static char ProjectDirectoryBuffer[VX_MAX_PROJECT_NAME_LENGTH];
		inline static char ProjectNameBuffer[VX_MAX_PROJECT_NAME_LENGTH];
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
		void CreateProject();

		void ResetInputFields();

		void ReplaceToken(std::string& str, const char* token, const std::string& value);
        void ResetWorkingDirectory();
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
