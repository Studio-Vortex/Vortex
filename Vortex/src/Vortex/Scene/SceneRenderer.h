#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math.h"
#include "Vortex/Core/UUID.h"
#include "Vortex/Scene/Components.h"

namespace Vortex {

	// Forward declarations
	class Scene;
	class Camera;
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
		static void RenderSkybox(const Math::mat4& view, const Math::mat4& projection, Scene* scene);
		static void SetMaterialFlags(const SharedRef<Material>& material);
		static void ResetAllMaterialFlags();
	};

}
