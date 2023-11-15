#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Scene.h"

#include "Vortex/Editor/EditorPanelTypes.h"

#include <string>

namespace Vortex {

	class VORTEX_API EditorPanel : public RefCounted
	{
	public:
		bool IsOpen = false;

	public:
		virtual ~EditorPanel() = default;

		virtual void OnEditorAttach() {}
		virtual void OnEditorDetach() {}

		virtual void OnGuiRender() = 0;

		static EditorPanelType GetStaticType() { return EditorPanelType::None; }
		virtual EditorPanelType GetPanelType() const { return GetStaticType(); }

		virtual void SetProjectContext(SharedReference<Project> project) { m_ContextProject = project; }
		virtual void SetSceneContext(SharedReference<Scene> scene) { m_ContextScene = scene; }

		virtual const std::string& GetName() const { return m_PanelName; }
		virtual void SetName(const std::string& name) { m_PanelName = name; }

	protected:
		SharedReference<Project> m_ContextProject = nullptr;
		SharedReference<Scene> m_ContextScene = nullptr;
		std::string m_PanelName;
	};

}
