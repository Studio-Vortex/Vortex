#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"
#include "Vortex/Core/Math/Math.h"

#include "Vortex/Scripting/ScriptFieldInstance.h"
#include "Vortex/Scripting/ManagedMethods.h"
#include "Vortex/Scripting/ScriptClass.h"

#include <unordered_map>
#include <string>

namespace Vortex {

	class Entity;
	struct Collision;

	class VORTEX_API ScriptInstance : public RefCounted
	{
	public:
		ScriptInstance() = default;
		ScriptInstance(SharedReference<ScriptClass>& scriptClass, Entity entity);
		~ScriptInstance() = default;

		void InvokeOnAwake();
		void InvokeOnCreate();
		void InvokeOnUpdate(float delta);
		void InvokeOnDestroy();
		void InvokeOnCollisionEnter(Collision& collision);
		void InvokeOnCollisionExit(Collision& collision);
		void InvokeOnTriggerEnter(Collision& collision);
		void InvokeOnTriggerExit(Collision& collision);
		void InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque);
		void InvokeOnRaycastCollision();
		void InvokeOnEnabled();
		void InvokeOnDisabled();
		void InvokeOnGui();

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

		std::unordered_map<ManagedMethod, MonoMethod*> m_ManagedMethods;

		inline static char s_FieldValueBuffer[VX_SCRIPT_FIELD_MAX_BYTES];

#ifdef VX_DEBUG
		std::string m_DebugName = "";
#endif

	private:
		friend class ScriptEngine;
		friend struct ScriptFieldInstance;
	};

}