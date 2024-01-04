#include "vxpch.h"
#include "ScriptInstance.h"

#include "Vortex/Scene/Actor.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/ScriptUtils.h"

namespace Vortex {

	ScriptInstance::ScriptInstance(SharedReference<ScriptClass>& scriptClass)
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
		m_ManagedMethods[ManagedMethod::OnEnable] = m_ScriptClass->GetMethod("OnEnabled", 0);
		m_ManagedMethods[ManagedMethod::OnDisable] = m_ScriptClass->GetMethod("OnDisabled", 0);
		m_ManagedMethods[ManagedMethod::OnDebugRender] = m_ScriptClass->GetMethod("OnDebugRender", 0);
		m_ManagedMethods[ManagedMethod::OnGui] = m_ScriptClass->GetMethod("OnGui", 0);

		SharedReference<ScriptClass> entityClass = ScriptEngine::GetCoreActorClass();
		m_ActorConstructor = entityClass->GetMethod(".ctor", 1);
	}

	void ScriptInstance::InvokeConstructor(UUID entityUUID)
	{
		void* param = (void*)&entityUUID;
		ScriptUtils::InvokeMethod(m_Instance, m_ActorConstructor, &param);
	}

	void ScriptInstance::InvokeOnAwake()
	{
		ManagedMethod method = ManagedMethod::OnAwake;
		if (!MethodExists(method))
		{
			return;
		}

		MonoMethod* onAwakeMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onAwakeMethod);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		ManagedMethod method = ManagedMethod::OnCreate;
		if (!MethodExists(method))
		{
			return;
		}

		MonoMethod* onCreateMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onCreateMethod);
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
		ManagedMethod method = ManagedMethod::OnDestroy;
		if (!MethodExists(method))
		{
			return;
		}

		MonoMethod* onDestroyMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onDestroyMethod);
	}

	void ScriptInstance::InvokeOnCollisionEnter(const Collision& collision)
	{
		ManagedMethod method = ManagedMethod::OnCollisionEnter;
		if (!MethodExists(method))
		{
			return;
		}

		void* param = (void*)&collision;
		MonoMethod* onCollisionEnterMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onCollisionEnterMethod, &param);
	}

	void ScriptInstance::InvokeOnCollisionExit(const Collision& collision)
	{
		ManagedMethod method = ManagedMethod::OnCollisionExit;
		if (!MethodExists(method))
		{
			return;
		}

		void* param = (void*)&collision;
		MonoMethod* onCollisionExitMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onCollisionExitMethod, &param);
	}

	void ScriptInstance::InvokeOnTriggerEnter(const Collision& collision)
	{
		ManagedMethod method = ManagedMethod::OnTriggerEnter;
		if (!MethodExists(method))
		{
			return;
		}

		void* param = (void*)&collision;
		MonoMethod* onTriggerEnterMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onTriggerEnterMethod, &param);
	}

	void ScriptInstance::InvokeOnTriggerExit(const Collision& collision)
	{
		ManagedMethod method = ManagedMethod::OnTriggerExit;
		if (!MethodExists(method))
		{
			return;
		}

		void* param = (void*)&collision;
		MonoMethod* onTriggerExitMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onTriggerExitMethod, &param);
	}

	void ScriptInstance::InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
	{
		ManagedMethod method = ManagedMethod::OnFixedJointDisconnected;
		if (!MethodExists(method))
		{
			return;
		}

		void* params[] = { (void*)&forceAndTorque.first, (void*)&forceAndTorque.second };
		MonoMethod* onFixedJointDisconnectedMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onFixedJointDisconnectedMethod, params);
	}

	void ScriptInstance::InvokeOnEnable()
	{
		ManagedMethod method = ManagedMethod::OnEnable;
		if (!MethodExists(method))
		{
			return;
		}

		MonoMethod* onEnabledMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onEnabledMethod);
	}

	void ScriptInstance::InvokeOnDisable()
	{
		ManagedMethod method = ManagedMethod::OnDisable;
		if (!MethodExists(method))
		{
			return;
		}

		MonoMethod* onDisabledMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onDisabledMethod);
	}

    void ScriptInstance::InvokeOnDebugRender()
    {
		ManagedMethod method = ManagedMethod::OnDebugRender;
		if (!MethodExists(method))
		{
			return;
		}

		MonoMethod* onDebugRenderMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onDebugRenderMethod);
    }

	void ScriptInstance::InvokeOnGui()
	{
		ManagedMethod method = ManagedMethod::OnGui;
		if (!MethodExists(method))
		{
			return;
		}

		MonoMethod* onGuiMethod = m_ManagedMethods[method];
		ScriptUtils::InvokeMethod(m_Instance, onGuiMethod);
	}

	bool ScriptInstance::MethodExists(ManagedMethod method) const
	{
		const bool contained = m_ManagedMethods.contains(method);
		if (!contained)
		{
			return false;
		}

		const MonoMethod* ptr = m_ManagedMethods.at(method);
		if (ptr == nullptr)
		{
			return false;
		}

		return true;
    }

	bool ScriptInstance::GetFieldValueInternal(const std::string& fieldName, void* buffer)
	{
		const std::map<std::string, ScriptField>& fields = m_ScriptClass->GetFields();
		auto it = fields.find(fieldName);

		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& fieldName, const void* value)
	{
		const std::map<std::string, ScriptField>& fields = m_ScriptClass->GetFields();
		auto it = fields.find(fieldName);

		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;
	}

}
