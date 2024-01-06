#include "vxpch.h"
#include "ScriptInstance.h"

#include "Vortex/Scene/Actor.h"

#include "Vortex/Scripting/ScriptUtils.h"

namespace Vortex {

	ScriptInstance::ScriptInstance(SharedReference<ScriptClass>& scriptClass)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_ManagedMethods[ManagedMethod::OnAwake] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnAwake), 0);
		m_ManagedMethods[ManagedMethod::OnCreate] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnCreate), 0);
		m_ManagedMethods[ManagedMethod::OnUpdateDelta] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnUpdate), 1);
		m_ManagedMethods[ManagedMethod::OnUpdate] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnUpdate), 0);
		m_ManagedMethods[ManagedMethod::OnDestroy] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnDestroy), 0);
		m_ManagedMethods[ManagedMethod::OnCollisionEnter] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnCollisionEnter), 1);
		m_ManagedMethods[ManagedMethod::OnCollisionExit] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnCollisionExit), 1);
		m_ManagedMethods[ManagedMethod::OnTriggerEnter] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnTriggerEnter), 1);
		m_ManagedMethods[ManagedMethod::OnTriggerExit] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnTriggerExit), 1);
		m_ManagedMethods[ManagedMethod::OnFixedJointDisconnected] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnFixedJointDisconnected), 2);
		m_ManagedMethods[ManagedMethod::OnEnable] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnEnable), 0);
		m_ManagedMethods[ManagedMethod::OnDisable] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnDisable), 0);
		m_ManagedMethods[ManagedMethod::OnDebugRender] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnDebugRender), 0);
		m_ManagedMethods[ManagedMethod::OnGuiRender] = m_ScriptClass->GetMethod(Utils::StringFromManagedMethod(ManagedMethod::OnGuiRender), 0);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnAwake()
	{
		ManagedMethod method = ManagedMethod::OnAwake;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		MonoMethod* onAwakeMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onAwakeMethod);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnCreate()
	{
		ManagedMethod method = ManagedMethod::OnCreate;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		MonoMethod* onCreateMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onCreateMethod);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnUpdate(TimeStep delta)
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
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onUpdateMethod, &param);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnDestroy()
	{
		ManagedMethod method = ManagedMethod::OnDestroy;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		MonoMethod* onDestroyMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onDestroyMethod);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnCollisionEnter(const Collision& collision)
	{
		ManagedMethod method = ManagedMethod::OnCollisionEnter;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		void* param = (void*)&collision;
		MonoMethod* onCollisionEnterMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onCollisionEnterMethod, &param);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnCollisionExit(const Collision& collision)
	{
		ManagedMethod method = ManagedMethod::OnCollisionExit;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		void* param = (void*)&collision;
		MonoMethod* onCollisionExitMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onCollisionExitMethod, &param);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnTriggerEnter(const Collision& collision)
	{
		ManagedMethod method = ManagedMethod::OnTriggerEnter;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		void* param = (void*)&collision;
		MonoMethod* onTriggerEnterMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onTriggerEnterMethod, &param);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnTriggerExit(const Collision& collision)
	{
		ManagedMethod method = ManagedMethod::OnTriggerExit;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		void* param = (void*)&collision;
		MonoMethod* onTriggerExitMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onTriggerExitMethod, &param);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
	{
		ManagedMethod method = ManagedMethod::OnFixedJointDisconnected;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		void* params[] = { (void*)&forceAndTorque.first, (void*)&forceAndTorque.second };
		MonoMethod* onFixedJointDisconnectedMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onFixedJointDisconnectedMethod, params);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnEnable()
	{
		ManagedMethod method = ManagedMethod::OnEnable;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		MonoMethod* onEnabledMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onEnabledMethod);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnDisable()
	{
		ManagedMethod method = ManagedMethod::OnDisable;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		MonoMethod* onDisabledMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onDisabledMethod);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnDebugRender()
	{
		ManagedMethod method = ManagedMethod::OnDebugRender;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		MonoMethod* onDebugRenderMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onDebugRenderMethod);
		return vxstd::make_option(result);
	}

	vxstd::option<RT_ScriptInvokeResult> ScriptInstance::InvokeOnGuiRender()
	{
		ManagedMethod method = ManagedMethod::OnGuiRender;
		if (!MethodExists(method))
		{
			return vxstd::make_option<RT_ScriptInvokeResult>();
		}

		MonoMethod* onGuiMethod = m_ManagedMethods[method];
		RT_ScriptInvokeResult result = ScriptUtils::InvokeMethod(m_Instance, onGuiMethod);
		return vxstd::make_option(result);
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
