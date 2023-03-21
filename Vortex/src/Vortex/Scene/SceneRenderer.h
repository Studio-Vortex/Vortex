#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"
#include "Vortex/Core/UUID.h"
#include "Vortex/Asset/Asset.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"

#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include <map>

namespace Vortex {

	class Scene;
	class Entity;
	class Camera;
	class Skybox;
	class Material;
	class Framebuffer;
	struct TransformComponent;
	struct SkyboxComponent;

	struct SceneRenderPacket
	{
		Camera* MainCamera = nullptr;
		Math::mat4 MainCameraWorldSpaceTransform = {};
		Math::vec3 MainCameraWorldSpaceTranslation = {};
		SharedRef<Framebuffer> TargetFramebuffer = nullptr;
		Scene* Scene = nullptr;
		bool EditorScene = false;
	};

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void RenderScene(const SceneRenderPacket& renderPacket);

	private:
		void FindCurrentEnvironment(const SceneRenderPacket& renderPacket, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment);
		void LightPass(const SceneRenderPacket& renderPacket);
		std::map<float, Entity> SortMeshGeometry(const SceneRenderPacket& renderPacket);
		void SortEntityByDistance(std::map<float, Entity>& sortedEntities, float distance, Entity entity, uint32_t offset = 0);
		void GeometryPass(const SceneRenderPacket& renderPacket, const std::map<float, Entity>& sortedEntities);
		void RenderMesh(Scene* scene, Entity entity, const SceneLightDescription& sceneLightDesc);
		void RenderStaticMesh(Scene* scene, Entity entity, const SceneLightDescription& sceneLightDesc);
		void SetEnvironment(AssetHandle environmentHandle, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment);
		void ClearEnvironment();

		void RenderEnvironment(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent* skyboxComponent, SharedReference<Skybox>& environment);
		void SetMaterialFlags(const SharedReference<Material>& material);
		void ResetAllMaterialFlags();
	};

}