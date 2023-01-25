#pragma once

#include <Vortex.h>

namespace Vortex {

	class ProjectSettingsPanel : public RefCounted
	{
	public:
		ProjectSettingsPanel(SharedRef<Project> project);

		void OnGuiRender();
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;

	private:
		ProjectProperties& m_Properties;
	};

}
