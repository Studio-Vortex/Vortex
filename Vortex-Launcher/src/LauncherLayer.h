#pragma once

#include <Vortex.h>

namespace Vortex {

#define VX_MAX_PROJECT_NAME_LENGTH 256
#define VX_MAX_PROJECT_DIR_LENGTH 256

	struct LauncherProperties
	{
		std::filesystem::path WorkingDirectory = "";
		std::filesystem::path ProjectPath = "";
		std::filesystem::path EditorPath = "";

		inline static char ProjectDirectoryBuffer[VX_MAX_PROJECT_NAME_LENGTH];
		inline static char ProjectNameBuffer[VX_MAX_PROJECT_NAME_LENGTH];

		ProjectType ProjectType = ProjectType::e3D;
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

		const char* ProjectTypeToString(ProjectType type);

	private:
		LauncherProperties m_Properties;
		SharedRef<Framebuffer> m_Framebuffer = nullptr;
		SharedRef<Texture2D> m_VortexLogoIcon = nullptr;
		SharedRef<Texture2D> m_SelectedProjectIcon = nullptr;
		Math::vec2 m_ViewportSize = Math::vec2();

		bool m_CreatingNewProject = false;
	};

}
