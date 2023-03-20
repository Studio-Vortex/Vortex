#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"
#include "Vortex/Core/UUID.h"
#include "Vortex/Scene/Components.h"

namespace Vortex {

	// Forward declarations
	class Scene;
	class Camera;
	class Skybox;
	class Material;
	class Framebuffer;
	struct TransformComponent;

	struct SceneRenderPacket
	{
		Camera* MainCamera = nullptr;
		TransformComponent MainCameraWorldSpaceTransform = {};
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
		static void RenderEnvironment(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment);
		static void SetMaterialFlags(const SharedReference<Material>& material);
		static void ResetAllMaterialFlags();
	};

}
