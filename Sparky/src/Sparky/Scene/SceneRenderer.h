#pragma once

#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Renderer/EditorCamera.h"
#include "Sparky/Core/Math.h"

#include <entt/entt.hpp>

namespace Sparky {

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void RenderFromSceneCamera(SceneCamera* sceneCamera, const Math::mat4& cameraTransform, entt::registry& sceneRegistry);
		void RenderFromEditorCamera(EditorCamera& editorCamera, entt::registry& sceneRegistry);
	};

}
