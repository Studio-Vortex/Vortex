#pragma once

#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

namespace Vortex {

	class EditorPanel : public RefCounted
	{
	public:
		EditorPanel() = default;
		virtual ~EditorPanel() = default;

		virtual void OnGuiRender() {}
		virtual void SetProjectContext(SharedRef<Project> project) {}
		virtual void SetSceneContext(SharedRef<Scene> scene) {}
	};

}
