#pragma once

#include <Vortex.h>

namespace Vortex {

	class AssetRegistryPanel
	{
	public:
		AssetRegistryPanel() = default;
		~AssetRegistryPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedReference<Project>& project) {}
		void SetSceneContext(SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderLoadedAssets();

	private:
		inline static bool s_ShowPanel = false;
		ImGuiTextFilter m_AssetSearchTextFilter;
	};

}
