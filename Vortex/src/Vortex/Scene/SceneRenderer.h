#pragma once

#include "Vortex/Core/Math.h"
#include "Vortex/Core/UUID.h"
#include "Vortex/Scene/Components.h"

namespace Vortex {

	// Forward declarations

	class Scene;
	class Camera;
	struct TransformComponent;

	struct SceneRenderPacket
	{
		Camera* MainCamera;
		TransformComponent CameraWorldSpaceTransform;
		Scene* Scene;
		bool EditorScene;
	};

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void RenderScene(const SceneRenderPacket& renderPacket);

		static void RenderSkybox(const Math::mat4& view, const Math::mat4& projection, Scene* scene);
	};

}
