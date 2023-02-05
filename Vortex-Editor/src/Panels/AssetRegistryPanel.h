#pragma once

#include <Vortex.h>

namespace Vortex {

	class AssetRegistryPanel
	{
	public:
		AssetRegistryPanel() = default;
		~AssetRegistryPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(SharedRef<Scene> scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderLoadedAssets();

	private:
		inline static bool s_ShowPanel = false;
	};

}
