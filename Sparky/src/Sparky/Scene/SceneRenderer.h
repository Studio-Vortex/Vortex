#pragma once

#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Renderer/EditorCamera.h"
#include "Sparky/Core/Math.h"

#include <entt/entt.hpp>

namespace Sparky {

	// Forward declaration
	struct TransformComponent;

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void RenderFromSceneCamera(SceneCamera* sceneCamera, const TransformComponent& cameraTransform, entt::registry& sceneRegistry);
		void RenderFromEditorCamera(EditorCamera& editorCamera, entt::registry& sceneRegistry);

		static void RenderSkybox(const Math::mat4& view, const Math::mat4& projection, entt::registry& sceneRegistry);
	};

}
