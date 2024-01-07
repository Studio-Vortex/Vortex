#include "vxpch.h"
#include "ScriptMethods.h"

namespace Vortex {

	namespace Utils {

		ScriptMethod ScriptMethodFromString(const std::string& str)
		{
			if (str == "OnAwake")                  return ScriptMethod::OnAwake;
			if (str == "OnEnable")                 return ScriptMethod::OnEnable;
			if (str == "OnCreate")                 return ScriptMethod::OnCreate;
			if (str == "OnUpdate")                 return ScriptMethod::OnUpdate;
			if (str == "OnPostUpdate")             return ScriptMethod::OnPostUpdate;
			if (str == "OnDebugRender")            return ScriptMethod::OnDebugRender;
			if (str == "OnCollisionEnter")         return ScriptMethod::OnCollisionEnter;
			if (str == "OnCollisionExit")          return ScriptMethod::OnCollisionExit;
			if (str == "OnTriggerEnter")           return ScriptMethod::OnTriggerEnter;
			if (str == "OnTriggerExit")            return ScriptMethod::OnTriggerExit;
			if (str == "OnFixedJointDisconnected") return ScriptMethod::OnFixedJointDisconnected;
			if (str == "OnApplicationQuit")        return ScriptMethod::OnApplicationQuit;
			if (str == "OnDisable")                return ScriptMethod::OnDisable;
			if (str == "OnDestroy")                return ScriptMethod::OnDestroy;
			if (str == "OnGuiRender")              return ScriptMethod::OnGuiRender;

			VX_CORE_ASSERT(false, "Unknown Managed Method!");
			return (ScriptMethod)0;
		}

		std::string StringFromScriptMethod(ScriptMethod method)
		{
			switch (method)
			{
				case ScriptMethod::OnAwake:                  return "OnAwake";
				case ScriptMethod::OnEnable:                 return "OnEnable";
				case ScriptMethod::OnCreate:                 return "OnCreate";
				case ScriptMethod::OnUpdate:                 return "OnUpdate";
				case ScriptMethod::OnPostUpdate:             return "OnPostUpdate";
				case ScriptMethod::OnDebugRender:            return "OnDebugRender";
				case ScriptMethod::OnCollisionEnter:         return "OnCollisionEnter";
				case ScriptMethod::OnCollisionExit:          return "OnCollisionExit";
				case ScriptMethod::OnTriggerEnter:           return "OnTriggerEnter";
				case ScriptMethod::OnTriggerExit:            return "OnTriggerExit";
				case ScriptMethod::OnFixedJointDisconnected: return "OnFixedJointDisconnected";
				case ScriptMethod::OnApplicationQuit:        return "OnApplicationQuit";
				case ScriptMethod::OnDisable:                return "OnDisable";
				case ScriptMethod::OnDestroy:                return "OnDestroy";
				case ScriptMethod::OnGuiRender:              return "OnGuiRender";
			}

			VX_CORE_ASSERT(false, "Unknown Managed Method!");
			return "";
		}

	}

}
