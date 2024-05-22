#include "PhysicsMaterialEditorPanel.h"

#include <Vortex/Editor/SelectionManager.h>

namespace Vortex {

	void PhysicsMaterialEditorPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		if (Actor selected = SelectionManager::GetSelectedActor())
		{
			RenderPhysicsMaterial(selected);
		}

		Gui::End();
	}

	void PhysicsMaterialEditorPanel::RenderPhysicsMaterial(Actor selectedEntity)
	{
		if (!selectedEntity.HasComponent<RigidBodyComponent>())
			return;

		if (!selectedEntity.HasAny<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, MeshColliderComponent>())
			return;

		UI::BeginPropertyGrid();

		AssetHandle physicsMaterialHandle = 0;
		SharedReference<PhysicsMaterial> physicsMaterial = nullptr;

		if (selectedEntity.HasComponent<BoxColliderComponent>())
		{
			physicsMaterialHandle = selectedEntity.GetComponent<BoxColliderComponent>().Material;
			if (AssetManager::IsHandleValid(physicsMaterialHandle))
			{
				physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(physicsMaterialHandle);
			}
		}
		else if (selectedEntity.HasComponent<SphereColliderComponent>())
		{
			physicsMaterialHandle = selectedEntity.GetComponent<SphereColliderComponent>().Material;
			if (AssetManager::IsHandleValid(physicsMaterialHandle))
			{
				physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(physicsMaterialHandle);
			}
		}
		else if (selectedEntity.HasComponent<CapsuleColliderComponent>())
		{
			physicsMaterialHandle = selectedEntity.GetComponent<CapsuleColliderComponent>().Material;
			if (AssetManager::IsHandleValid(physicsMaterialHandle))
			{
				physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(physicsMaterialHandle);
			}
		}
		else if (selectedEntity.HasComponent<MeshColliderComponent>())
		{
			physicsMaterialHandle = selectedEntity.GetComponent<MeshColliderComponent>().Material;
			if (AssetManager::IsHandleValid(physicsMaterialHandle))
			{
				physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(physicsMaterialHandle);
			}
		}

		if (physicsMaterial)
		{
			UI::Property("Name", physicsMaterial->Name);
			UI::Draw::Underline();
			Gui::Spacing();

			UI::Property("Static Friction", physicsMaterial->StaticFriction, 0.01f, 0.01f, 1.0f);
			UI::Property("Dynamic Friction", physicsMaterial->DynamicFriction, 0.01f, 0.01f, 1.0f);
			UI::Property("Bounciness", physicsMaterial->Bounciness, 0.01f, 0.01f, 1.0f);

			const char* combineModes[] = { "Average", "Maximum", "Minimum", "Multiply" };
			int32_t currentFrictionCombineMode = (uint32_t)physicsMaterial->FrictionCombineMode;
			if (UI::PropertyDropdown("Friction Combine Mode", combineModes, VX_ARRAYSIZE(combineModes), currentFrictionCombineMode))
			{
				physicsMaterial->FrictionCombineMode = (CombineMode)currentFrictionCombineMode;
			}

			int32_t currentBouncinessCombineMode = (uint32_t)physicsMaterial->BouncinessCombineMode;
			if (UI::PropertyDropdown("Bounciness Combine Mode", combineModes, VX_ARRAYSIZE(combineModes), currentBouncinessCombineMode))
			{
				physicsMaterial->BouncinessCombineMode = (CombineMode)currentBouncinessCombineMode;
			}
		}
		else
		{
			static std::string buffer;
			UI::Property("Material Name", buffer);

			if (Gui::Button("Create Physics Material"))
			{
				if (buffer.size() > 0)
				{
					AssetHandle assetHandle = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>(buffer, 0.6f, 0.6f, 0.0f);
					physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(assetHandle);

					buffer.clear();

					if (selectedEntity.HasComponent<BoxColliderComponent>())
					{
						selectedEntity.GetComponent<BoxColliderComponent>().Material = assetHandle;
					}
					else if (selectedEntity.HasComponent<SphereColliderComponent>())
					{
						selectedEntity.GetComponent<SphereColliderComponent>().Material = assetHandle;
					}
					else if (selectedEntity.HasComponent<CapsuleColliderComponent>())
					{
						selectedEntity.GetComponent<CapsuleColliderComponent>().Material = assetHandle;
					}
					else if (selectedEntity.HasComponent<MeshColliderComponent>())
					{
						selectedEntity.GetComponent<MeshColliderComponent>().Material = assetHandle;
					}
				}
			}
		}

		UI::EndPropertyGrid();
	}

}
