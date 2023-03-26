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
		m_ManagedMethods[ManagedMethod::OnRaycastCollision] = m_ScriptClass->GetMethod("OnRaycastCollision", 0);
		m_ManagedMethods[ManagedMethod::OnEnabled] = m_ScriptClass->GetMethod("OnEnabled", 0);
		m_ManagedMethods[ManagedMethod::OnDisabled] = m_ScriptClass->GetMethod("OnDisabled", 0);
		m_ManagedMethods[ManagedMethod::OnGui] = m_ScriptClass->GetMethod("OnGui", 0);

		// Call C# Entity constructor
		ScriptEngine::ConstructEntityRuntime(entity.GetUUID(), m_Instance);
	}

	void ScriptInstance::InvokeOnAwake()
	{
		if (!m_ManagedMethods[ManagedMethod::OnAwake])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnAwake]);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (!m_ManagedMethods[ManagedMethod::OnCreate])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnCreate]);
	}

	void ScriptInstance::InvokeOnUpdate(float delta)
	{
		MonoMethod* onUpdateMethod = nullptr;

		if (m_ManagedMethods[ManagedMethod::OnUpdateDelta])
		{
			void* param = &delta;
			ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnUpdateDelta], &param);
		}
		else if (m_ManagedMethods[ManagedMethod::OnUpdate])
		{
			ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnUpdate]);
		}
	}

	void ScriptInstance::InvokeOnDestroy()
	{
		if (!m_ManagedMethods[ManagedMethod::OnDestroy])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnDestroy]);
	}

	void ScriptInstance::InvokeOnCollisionEnter(Collision& collision)
	{
		if (!m_ManagedMethods[ManagedMethod::OnCollisionEnter])
			return;

		void* param = &collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnCollisionEnter], &param);
	}

	void ScriptInstance::InvokeOnCollisionExit(Collision& collision)
	{
		if (!m_ManagedMethods[ManagedMethod::OnCollisionExit])
			return;

		void* param = &collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnCollisionExit], &param);
	}

	void ScriptInstance::InvokeOnTriggerEnter(Collision& collision)
	{
		if (!m_ManagedMethods[ManagedMethod::OnTriggerEnter])
			return;

		void* param = &collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnTriggerEnter], &param);
	}

	void ScriptInstance::InvokeOnTriggerExit(Collision& collision)
	{
		if (!m_ManagedMethods[ManagedMethod::OnTriggerExit])
			return;

		void* param = &collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnTriggerExit], &param);
	}

	void ScriptInstance::InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
	{
		if (!m_ManagedMethods[ManagedMethod::OnFixedJointDisconnected])
			return;

		void* params[] = { (void*)&forceAndTorque.first, (void*)&forceAndTorque.second };
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnFixedJointDisconnected], params);
	}

	void ScriptInstance::InvokeOnRaycastCollision()
	{
		if (!m_ManagedMethods[ManagedMethod::OnRaycastCollision])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnRaycastCollision]);
	}

	void ScriptInstance::InvokeOnEnabled()
	{
		if (!m_ManagedMethods[ManagedMethod::OnEnabled])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnEnabled]);
	}

	void ScriptInstance::InvokeOnDisabled()
	{
		if (!m_ManagedMethods[ManagedMethod::OnDisabled])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnDisabled]);
	}

	void ScriptInstance::InvokeOnGui()
	{
		if (!m_ManagedMethods[ManagedMethod::OnGui])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnGui]);
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
