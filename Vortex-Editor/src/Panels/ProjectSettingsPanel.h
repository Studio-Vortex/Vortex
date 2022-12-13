#pragma once

#include <Vortex.h>

namespace Vortex {

	class ProjectSettingsPanel
	{
	public:
		ProjectSettingsPanel(const SharedRef<Project>& project);

		void OnGuiRender(bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;

	private:
		ProjectProperties& m_Properties;
	};

}
