#include "vxpch.h"
#include "ScriptInstance.h"

#include "Vortex/Scene/Entity.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/ScriptUtils.h"

namespace Vortex {

	ScriptInstance::ScriptInstance(SharedReference<ScriptClass>& scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_ManagedMethods[ManagedMethod::OnAwake] = m_ScriptClass->GetMethod("OnAwake", 0);
		m_ManagedMethods[ManagedMethod::OnCreate] = m_ScriptClass->GetMethod("OnCreate", 0);
		m_ManagedMethods[ManagedMethod::OnUpdateDelta] = m_ScriptClass->GetMethod("OnUpdate", 1);
		m_ManagedMethods[ManagedMethod::OnUpdate] = m_ScriptClass->GetMethod("OnUpdate", 0);
		m_ManagedMethods[ManagedMethod::OnDestroy] = m_ScriptClass->GetMethod("OnDestroy", 0);
		m_ManagedMethods[ManagedMethod::OnCollisionEnter] = m_ScriptClass->GetMethod("OnCollisionEnter", 1);
		m_ManagedMethods[ManagedMethod::OnCollisionExit] = m_ScriptClass->GetMethod("OnCollisionExit", 1);
		m_ManagedMethods[ManagedMethod::OnTriggerEnter] = m_ScriptClass->GetMethod("OnTriggerEnter", 1);
		m_ManagedMethods[ManagedMethod::OnTriggerExit] = m_ScriptClass->GetMethod("OnTriggerExit", 1);
		m_ManagedMethods[ManagedMethod::OnFixedJointDisconnected] = m_ScriptClass->GetMethod("OnFixedJointDisconnected", 2);
		m_ManagedMethods[ManagedMethod::OnEnabled] = m_ScriptClass->GetMethod("OnEnabled", 0);
		m_ManagedMethods[ManagedMethod::OnDisabled] = m_ScriptClass->GetMethod("OnDisabled", 0);
		m_ManagedMethods[ManagedMethod::OnGui] = m_ScriptClass->GetMethod("OnGui", 0);

		// Call C# Entity constructor
		ScriptEngine::EntityConstructorRuntime(entity.GetUUID(), m_Instance);
	}

	void ScriptInstance::InvokeOnAwake()
	{
		if (!MethodExists(ManagedMethod::OnAwake))
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnAwake]);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (!MethodExists(ManagedMethod::OnCreate))
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnCreate]);
	}

	void ScriptInstance::InvokeOnUpdate(TimeStep delta)
	{
		MonoMethod* onUpdateMethod = nullptr;
		void* param = nullptr;

		if (MethodExists(ManagedMethod::OnUpdateDelta))
		{
			float dt = delta.GetDeltaTime();
			param = &dt;
			onUpdateMethod = m_ManagedMethods[ManagedMethod::OnUpdateDelta];
		}
		else if (MethodExists(ManagedMethod::OnUpdate))
		{
			onUpdateMethod = m_ManagedMethods[ManagedMethod::OnUpdate];
		}

		// No OnUpdate method was found so just leave
		if (onUpdateMethod == nullptr)
		{
			return;
		}

		ScriptUtils::InvokeMethod(m_Instance, onUpdateMethod, &param);
	}

	void ScriptInstance::InvokeOnDestroy()
	{
		if (!MethodExists(ManagedMethod::OnDestroy))
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnDestroy]);
	}

	void ScriptInstance::InvokeOnCollisionEnter(const Collision& collision)
	{
		if (!MethodExists(ManagedMethod::OnCollisionEnter))
			return;

		void* param = (void*)&collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnCollisionEnter], &param);
	}

	void ScriptInstance::InvokeOnCollisionExit(const Collision& collision)
	{
		if (!MethodExists(ManagedMethod::OnCollisionExit))
			return;

		void* param = (void*)&collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnCollisionExit], &param);
	}

	void ScriptInstance::InvokeOnTriggerEnter(const Collision& collision)
	{
		if (!MethodExists(ManagedMethod::OnTriggerEnter))
			return;

		void* param = (void*)&collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnTriggerEnter], &param);
	}

	void ScriptInstance::InvokeOnTriggerExit(const Collision& collision)
	{
		if (!MethodExists(ManagedMethod::OnTriggerExit))
			return;

		void* param = (void*)&collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnTriggerExit], &param);
	}

	void ScriptInstance::InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
	{
		if (!MethodExists(ManagedMethod::OnFixedJointDisconnected))
			return;

		void* params[] = { (void*)&forceAndTorque.first, (void*)&forceAndTorque.second };
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnFixedJointDisconnected], params);
	}

	void ScriptInstance::InvokeOnEnabled()
	{
		if (!MethodExists(ManagedMethod::OnEnabled))
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnEnabled]);
	}

	void ScriptInstance::InvokeOnDisabled()
	{
		if (!MethodExists(ManagedMethod::OnDisabled))
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnDisabled]);
	}

	void ScriptInstance::InvokeOnGui()
	{
		if (!MethodExists(ManagedMethod::OnGui))
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnGui]);
	}

	bool ScriptInstance::MethodExists(ManagedMethod method) const
	{
		const bool contained = m_ManagedMethods.contains(method);
		const bool ptr = (bool)m_ManagedMethods.at(method);
		return contained && ptr;
    }

	bool ScriptInstance::GetFieldValueInternal(const std::string& fieldName, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(fieldName);

		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& fieldName, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(fieldName);

		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;
	}

}
