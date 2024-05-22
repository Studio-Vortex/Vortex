#pragma once

#include <Vortex.h>

namespace Vortex {

#define VX_MAX_PROJECT_NAME_LENGTH 256
#define VX_MAX_PROJECT_DIR_LENGTH 256

	struct LauncherProperties
	{
		Fs::Path WorkingDirectory = "";
		Fs::Path ProjectFilepath = "";
		Fs::Path EditorAppFilepath = "";

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

		void CreateProjectFilesAndDirectories();
		void CreatePremakeBuildScript();
		void GenerateProjectSolution();
		void BuildProjectSolution();

		void SaveProjectToDisk();
		void LaunchEditorApp();

		void ResetInputFields();
		void ClearProjectNameBuffer();
		void ClearProjectDirectoryBuffer();
		void ResetWorkingDirectory();

	private:
		LauncherProperties m_Properties;
		SharedReference<Framebuffer> m_Framebuffer = nullptr;
		SharedReference<Texture2D> m_SelectedProjectIcon = nullptr;

		Math::vec2 m_ViewportSize = Math::vec2();

		bool m_IsCreatingNewProject = false;
	};

}
