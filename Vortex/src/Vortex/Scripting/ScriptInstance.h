#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/TimeStep.h"

#include "Vortex/Math/Math.h"

#include "Vortex/Scene/Actor.h"

#include "Vortex/Scripting/ScriptClass.h"
#include "Vortex/Scripting/ScriptMethods.h"
#include "Vortex/Scripting/ScriptFieldInstance.h"
#include "Vortex/Scripting/RT_ScriptInvokeResult.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

#include "Vortex/stl/option.h"

#include <unordered_map>
#include <string>

namespace Vortex {

	struct Collision;

	class VORTEX_API ScriptInstance : public RefCounted
	{
	public:
		ScriptInstance() = default;
		ScriptInstance(UUID actorUUID, SharedReference<ScriptClass>& scriptClass);
		~ScriptInstance() = default;

		vxstl::option<RT_ScriptInvokeResult> InvokeOnAwake();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnEnable();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnReset();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnCreate();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnUpdate();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnPostUpdate();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnApplicationPause();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnApplicationResume();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnDebugRender();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnCollisionEnter(const Collision& collision);
		vxstl::option<RT_ScriptInvokeResult> InvokeOnCollisionExit(const Collision& collision);
		vxstl::option<RT_ScriptInvokeResult> InvokeOnTriggerEnter(const Collision& collision);
		vxstl::option<RT_ScriptInvokeResult> InvokeOnTriggerExit(const Collision& collision);
		vxstl::option<RT_ScriptInvokeResult> InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque);
		vxstl::option<RT_ScriptInvokeResult> InvokeOnApplicationQuit();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnDisable();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnDestroy();
		vxstl::option<RT_ScriptInvokeResult> InvokeOnGuiRender();

		bool ScriptMethodExists(ScriptMethod method) const;

		VX_FORCE_INLINE SharedReference<ScriptClass> GetScriptClass() { return m_ScriptClass; }
		VX_FORCE_INLINE const SharedReference<ScriptClass>& GetScriptClass() const { return m_ScriptClass; }

		template <typename TFieldType>
		VX_FORCE_INLINE TFieldType GetFieldValue(const std::string& fieldName)
		{
			static_assert(sizeof(TFieldType) <= VX_SCRIPT_FIELD_MAX_BYTES, "Type too large!");

			const bool found = GetFieldValueInternal(fieldName, s_FieldValueBuffer);
			if (!found)
				return TFieldType();

			return *(TFieldType*)s_FieldValueBuffer;
		}

		template <typename TFieldType>
		VX_FORCE_INLINE void SetFieldValue(const std::string& fieldName, TFieldType value)
		{
			static_assert(sizeof(TFieldType) <= VX_SCRIPT_FIELD_MAX_BYTES, "Type too large!");

			SetFieldValueInternal(fieldName, &value);
		}

		VX_FORCE_INLINE UUID GetActorUUID() const { return m_ActorUUID; }

		VX_FORCE_INLINE MonoObject* GetManagedObject() const { return m_Instance; }

	private:
		vxstl::option<RT_ScriptInvokeResult> InvokeParameteredMethodInternal(ScriptMethod method, void** params);

		bool GetFieldValueInternal(const std::string& fieldName, void* buffer);
		bool SetFieldValueInternal(const std::string& fieldName, const void* value);

	private:
		SharedReference<ScriptClass> m_ScriptClass = nullptr;
		std::unordered_map<ScriptMethod, MonoMethod*> m_ScriptMethods;

		UUID m_ActorUUID;

		MonoObject* m_Instance = nullptr;

	private:
		VX_FORCE_INLINE static char s_FieldValueBuffer[VX_SCRIPT_FIELD_MAX_BYTES];

	private:
		friend class ScriptEngine;
	};

}
