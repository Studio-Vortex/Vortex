#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include "Vortex/System/SystemTypes.h"

namespace Vortex {

#define SYSTEM_CLASS_TYPE(type) static SystemType GetStaticType() { return SystemType::##type; }\
								virtual SystemType GetSystemType() const override { return GetStaticType(); }

	class Scene;

	class VORTEX_API ISystem : public RefCounted
	{
	public:
		ISystem(const std::string& name = "Engine System") { m_DebugName = name; }
		virtual ~ISystem() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		static SystemType GetStaticType() { return SystemType::None; }
		virtual SystemType GetSystemType() const = 0;

		virtual void OnContextSceneCreated(Scene* context) = 0;
		virtual void OnContextSceneDestroyed(Scene* context) = 0;

		virtual void OnRuntimeStart(Scene* context) = 0;
		virtual void OnUpdateRuntime(Scene* context) = 0;
		virtual void OnRuntimeScenePaused(Scene* context) = 0;
		virtual void OnRuntimeSceneResumed(Scene* context) = 0;
		virtual void OnRuntimeStop(Scene* context) = 0;

		// Debug

		inline const std::string& GetDebugName() const { return m_DebugName; }

		virtual void OnGuiRender() = 0;

	private:
		std::string m_DebugName = "";
	};

}