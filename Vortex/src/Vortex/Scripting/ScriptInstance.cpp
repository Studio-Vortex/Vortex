#include "vxpch.h"
#include "ScriptInstance.h"

#include "Vortex/Scene/Actor.h"

#include "Vortex/Scripting/ScriptUtils.h"

namespace Vortex {

	ScriptInstance::ScriptInstance(SharedReference<ScriptClass>& scriptClass)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_ScriptMethods[ScriptMethod::OnAwake] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnAwake), 0);
		m_ScriptMethods[ScriptMethod::OnCreate] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnCreate), 0);
		m_ScriptMethods[ScriptMethod::OnUpdateDelta] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnUpdate), 1);
		m_ScriptMethods[ScriptMethod::OnUpdate] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnUpdate), 0);
		m_ScriptMethods[ScriptMethod::OnDestroy] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnDestroy), 0);
		m_ScriptMethods[ScriptMethod::OnCollisionEnter] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnCollisionEnter), 1);
		m_ScriptMethods[ScriptMethod::OnCollisionExit] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnCollisionExit), 1);
		m_ScriptMethods[ScriptMethod::OnTriggerEnter] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnTriggerEnter), 1);
		m_ScriptMethods[ScriptMethod::OnTriggerExit] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnTriggerExit), 1);
		m_ScriptMethods[ScriptMethod::OnFixedJointDisconnected] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnFixedJointDisconnected), 2);
		m_ScriptMethods[ScriptMethod::OnEnable] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnEnable), 0);
		m_ScriptMethods[ScriptMethod::OnDisable] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnDisable), 0);
		m_ScriptMethods[ScriptMethod::OnDebugRender] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnDebugRender), 0);
		m_ScriptMethods[ScriptMethod::OnGuiRender] = m_ScriptClass->GetMethod(Utils::StringFromScriptMethod(ScriptMethod::OnGuiRender), 0);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnAwake()
	{
		ScriptMethod method = ScriptMethod::OnAwake;
		return InvokeParameteredMethodInternal(method, nullptr);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnCreate()
	{
		ScriptMethod method = ScriptMethod::OnCreate;
		return InvokeParameteredMethodInternal(method, nullptr);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnUpdate(TimeStep delta)
	{
		MonoMethod* onUpdateMethod = nullptr;
		void* param = nullptr;

		if (ScriptMethodExists(ScriptMethod::OnUpdateDelta))
		{
			float dt = delta.GetDeltaTime();
			param = &dt;
			onUpdateMethod = m_ScriptMethods[ScriptMethod::OnUpdateDelta];
		}
		else if (ScriptMethodExists(ScriptMethod::OnUpdate))
		{
			onUpdateMethod = m_ScriptMethods[ScriptMethod::OnUpdate];
		}

		// No OnUpdate method was found so just leave
		if (onUpdateMethod == nullptr)
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		RT_ScriptInvokeResult result = ScriptUtils::InvokeManagedMethod(m_Instance, onUpdateMethod, &param);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnDestroy()
	{
		ScriptMethod method = ScriptMethod::OnDestroy;
		return InvokeParameteredMethodInternal(method, nullptr);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnCollisionEnter(const Collision& collision)
	{
		ScriptMethod method = ScriptMethod::OnCollisionEnter;
		void* param = (void*)&collision;
		return InvokeParameteredMethodInternal(method, &param);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnCollisionExit(const Collision& collision)
	{
		ScriptMethod method = ScriptMethod::OnCollisionExit;
		void* param = (void*)&collision;
		return InvokeParameteredMethodInternal(method, &param);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnTriggerEnter(const Collision& collision)
	{
		ScriptMethod method = ScriptMethod::OnTriggerEnter;
		void* param = (void*)&collision;
		return InvokeParameteredMethodInternal(method, &param);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnTriggerExit(const Collision& collision)
	{
		ScriptMethod method = ScriptMethod::OnTriggerExit;
		void* param = (void*)&collision;
		return InvokeParameteredMethodInternal(method, &param);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
	{
		ScriptMethod method = ScriptMethod::OnFixedJointDisconnected;
		void* params[] = { (void*)&forceAndTorque.first, (void*)&forceAndTorque.second };
		return InvokeParameteredMethodInternal(method, params);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnEnable()
	{
		ScriptMethod method = ScriptMethod::OnEnable;
		return InvokeParameteredMethodInternal(method, nullptr);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnDisable()
	{
		ScriptMethod method = ScriptMethod::OnDisable;
		return InvokeParameteredMethodInternal(method, nullptr);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnDebugRender()
	{
		ScriptMethod method = ScriptMethod::OnDebugRender;
		return InvokeParameteredMethodInternal(method, nullptr);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnGuiRender()
	{
		ScriptMethod method = ScriptMethod::OnGuiRender;
		return InvokeParameteredMethodInternal(method, nullptr);
	}

	bool ScriptInstance::ScriptMethodExists(ScriptMethod method) const
	{
		const bool contained = m_ScriptMethods.contains(method);
		if (!contained)
		{
			return false;
		}

		const MonoMethod* ptr = m_ScriptMethods.at(method);
		if (ptr == nullptr)
		{
			return false;
		}

		return true;
    }

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeParameteredMethodInternal(ScriptMethod method, void** params)
	{
		if (!m_Instance)
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		if (!ScriptMethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		MonoMethod* managedMethod = m_ScriptMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeManagedMethod(m_Instance, managedMethod, params);
		return vxstd::make_option(result);
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
