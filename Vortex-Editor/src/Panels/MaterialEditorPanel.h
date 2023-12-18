#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class MaterialEditorPanel : public EditorPanel
	{
	public:
		~MaterialEditorPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(MaterialEditor)

	private:
		void RenderMeshMaterial(Actor selectedActor);
		void ParameterCallback(SharedReference<Material> material, uint32_t materialIndex);
		SharedReference<Texture2D> GetMaterialTexture(SharedReference<Material> material, uint32_t index);
		void SetMaterialTexture(SharedReference<Material> material, AssetHandle textureHandle, uint32_t index);
		void RenderMaterialFlags(SharedReference<Material> material);
		void RenderMaterialProperties(SharedReference<Material> material);
		bool MaterialTextureHasProperties(uint32_t index);
		using MaterialParameterCallbackFunc = const std::function<void(SharedReference<Material>, uint32_t)>&;
		void RenderMaterialTexturesAndProperties(SharedReference<Material> material, MaterialParameterCallbackFunc parameterCallback);

	private:
		ImGuiTextFilter m_ShaderDropdownTextFilter;
	};

}
