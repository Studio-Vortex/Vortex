#pragma once

#include "Sparky/Core/Math.h"

namespace Sparky {

	// Forward declarations

	class Scene;
	class SceneCamera;
	class EditorCamera;
	struct TransformComponent;

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void RenderFromSceneCamera(SceneCamera& sceneCamera, const TransformComponent& cameraTransform, Scene* scene);
		void RenderFromEditorCamera(EditorCamera& editorCamera, Scene* scene);

		static void RenderSkybox(const Math::mat4& view, const Math::mat4& projection, Scene* scene);
	};

}
