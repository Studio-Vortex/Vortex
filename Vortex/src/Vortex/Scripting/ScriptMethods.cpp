#include "vxpch.h"
#include "ScriptMethods.h"

namespace Vortex {

	namespace Utils {

		ScriptMethod ScriptMethodFromString(const std::string& str)
		{
			if (str == "OnAwake")                  return ScriptMethod::OnAwake;
			if (str == "OnCreate")                 return ScriptMethod::OnCreate;
			if (str == "OnUpdateDelta")            return ScriptMethod::OnUpdateDelta;
			if (str == "OnUpdate")                 return ScriptMethod::OnUpdate;
			if (str == "OnDestroy")                return ScriptMethod::OnDestroy;
			if (str == "OnCollisionEnter")         return ScriptMethod::OnCollisionEnter;
			if (str == "OnCollisionExit")          return ScriptMethod::OnCollisionExit;
			if (str == "OnTriggerEnter")           return ScriptMethod::OnTriggerEnter;
			if (str == "OnTriggerExit")            return ScriptMethod::OnTriggerExit;
			if (str == "OnFixedJointDisconnected") return ScriptMethod::OnFixedJointDisconnected;
			if (str == "OnEnable")                 return ScriptMethod::OnEnable;
			if (str == "OnDisable")                return ScriptMethod::OnDisable;
			if (str == "OnDebugRender")            return ScriptMethod::OnDebugRender;
			if (str == "OnGuiRender")              return ScriptMethod::OnGuiRender;

			VX_CORE_ASSERT(false, "Unknown Managed Method!");
			return (ScriptMethod)0;
		}

		std::string StringFromScriptMethod(ScriptMethod method)
		{
			switch (method)
			{
				case ScriptMethod::OnAwake:                  return "OnAwake";
				case ScriptMethod::OnCreate:                 return "OnCreate";
				case ScriptMethod::OnUpdateDelta:            return "OnUpdateDelta";
				case ScriptMethod::OnUpdate:                 return "OnUpdate";
				case ScriptMethod::OnDestroy:                return "OnDestroy";
				case ScriptMethod::OnCollisionEnter:         return "OnCollisionEnter";
				case ScriptMethod::OnCollisionExit:          return "OnCollisionExit";
				case ScriptMethod::OnTriggerEnter:           return "OnTriggerEnter";
				case ScriptMethod::OnTriggerExit:            return "OnTriggerExit";
				case ScriptMethod::OnFixedJointDisconnected: return "OnFixedJointDisconnected";
				case ScriptMethod::OnEnable:                 return "OnEnable";
				case ScriptMethod::OnDisable:                return "OnDisable";
				case ScriptMethod::OnDebugRender:            return "OnDebugRender";
				case ScriptMethod::OnGuiRender:              return "OnGuiRender";
			}

			VX_CORE_ASSERT(false, "Unknown Managed Method!");
			return "";
		}

	}

}
