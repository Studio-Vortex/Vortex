#pragma once

#include <Sparky.h>

namespace Sparky {

	class MaterialViewerPanel
	{
	public:
		MaterialViewerPanel() = default;

		void OnGuiRender(bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }
		void SetTexture(const SharedRef<Texture2D>& texture);

	private:
		SharedRef<Texture2D> m_CurrentTexture = nullptr;
		inline static bool s_ShowPanel = false;
	};

}
