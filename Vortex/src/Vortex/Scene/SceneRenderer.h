#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"

#include "Vortex/Math/Math.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

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

	struct VORTEX_API SceneRenderPacket
	{
		Camera* MainCamera = nullptr;
		Math::mat4 MainCameraViewMatrix = {};
		Math::mat4 MainCameraProjectionMatrix = {};
		Math::vec3 MainCameraWorldSpaceTranslation = {};
		SharedReference<Framebuffer> TargetFramebuffer = nullptr;
		Scene* Scene = nullptr;
		bool EditorScene = false;
	};

	class VORTEX_API SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void RenderScene(const SceneRenderPacket& renderPacket);

	private:
		void OnRenderScene2D(const SceneRenderPacket& renderPacket);
		void OnRenderScene3D(const SceneRenderPacket& renderPacket);

		// 2D

		void BeginScene2D(const SceneRenderPacket& renderPacket);
		void EndScene2D();

		void LightPass2D(const SceneRenderPacket& renderPacket);
		void SpritePass2D(const SceneRenderPacket& renderPacket);
		void ParticlePass2D(const SceneRenderPacket& renderPacket);
		void TextPass2D(const SceneRenderPacket& renderPacket);
		void SceneGizmosPass2D(const SceneRenderPacket& renderPacket);

		// 3D

		void BeginScene(const SceneRenderPacket& renderPacket);
		void EndScene();

		void LightPass(const SceneRenderPacket& renderPacket);
		void SortMeshGeometry(const SceneRenderPacket& renderPacket, std::map<float, Entity>& sortedGeometry);
		void SortEntityByDistance(std::map<float, Entity>& sortedEntities, float distance, Entity entity, uint32_t offset = 0);
		void GeometryPass(const SceneRenderPacket& renderPacket, const std::map<float, Entity>& sortedGeometry);
		void RenderMesh(Scene* scene, Entity entity, const SceneLightDescription& sceneLightDesc);
		void RenderStaticMesh(Scene* scene, Entity entity, const SceneLightDescription& sceneLightDesc);

		// Environment

		void FindCurrentEnvironment(const SceneRenderPacket& renderPacket, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment);
		void SetEnvironment(AssetHandle environmentHandle, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment);
		void ClearEnvironment();
		void RenderEnvironment(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent* skyboxComponent, SharedReference<Skybox>& environment);

		// Helpers

		void SetMaterialFlags(const SharedReference<Material>& material);
		void ResetMaterialFlags();

	private:
		std::mutex m_GeometrySortMutex;
	};

}
