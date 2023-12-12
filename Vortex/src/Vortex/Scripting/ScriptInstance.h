#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/TimeStep.h"

#include "Vortex/Math/Math.h"

#include "Vortex/Scene/Entity.h"

#include "Vortex/Scripting/ScriptClass.h"
#include "Vortex/Scripting/ManagedMethods.h"
#include "Vortex/Scripting/ScriptFieldInstance.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

#include <unordered_map>
#include <string>

namespace Vortex {

	struct Collision;

	class VORTEX_API ScriptInstance : public RefCounted
	{
	public:
		ScriptInstance() = default;
		ScriptInstance(SharedReference<ScriptClass>& scriptClass, Entity entity);
		~ScriptInstance() = default;

		void InvokeConstructor(UUID entityUUID);

		void InvokeOnAwake();
		void InvokeOnCreate();
		void InvokeOnUpdate(TimeStep delta);
		void InvokeOnDestroy();
		void InvokeOnCollisionEnter(const Collision& collision);
		void InvokeOnCollisionExit(const Collision& collision);
		void InvokeOnTriggerEnter(const Collision& collision);
		void InvokeOnTriggerExit(const Collision& collision);
		void InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque);
		void InvokeOnEnable();
		void InvokeOnDisable();
		void InvokeOnGui();

		bool MethodExists(ManagedMethod method) const;

		VX_FORCE_INLINE SharedReference<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		template <typename TFieldType>
		TFieldType GetFieldValue(const std::string& fieldName)
		{
			static_assert(sizeof(TFieldType) <= VX_SCRIPT_FIELD_MAX_BYTES, "Type too large!");

			const bool found = GetFieldValueInternal(fieldName, s_FieldValueBuffer);
			if (!found)
				return TFieldType();

			return *(TFieldType*)s_FieldValueBuffer;
		}

		template <typename TFieldType>
		void SetFieldValue(const std::string& fieldName, TFieldType value)
		{
			static_assert(sizeof(TFieldType) <= VX_SCRIPT_FIELD_MAX_BYTES, "Type too large!");

			SetFieldValueInternal(fieldName, &value);
		}

		MonoObject* GetManagedObject() const { return m_Instance; }

	private:
		bool GetFieldValueInternal(const std::string& fieldName, void* buffer);
		bool SetFieldValueInternal(const std::string& fieldName, const void* value);

	private:
		SharedReference<ScriptClass> m_ScriptClass = nullptr;

		MonoObject* m_Instance = nullptr;

		MonoMethod* m_EntityConstructor = nullptr;
		std::unordered_map<ManagedMethod, MonoMethod*> m_ManagedMethods;

		inline static char s_FieldValueBuffer[VX_SCRIPT_FIELD_MAX_BYTES];

	private:
		friend class ScriptEngine;
	};

}
