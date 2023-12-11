#include "vxpch.h"
#include "ManagedMethods.h"

namespace Vortex {

	namespace Utils {

		ManagedMethod ManagedMethodFromString(const std::string& str)
		{
			if (str == "OnAwake")                  return ManagedMethod::OnAwake;
			if (str == "OnCreate")                 return ManagedMethod::OnCreate;
			if (str == "OnUpdateDelta")            return ManagedMethod::OnUpdateDelta;
			if (str == "OnUpdate")                 return ManagedMethod::OnUpdate;
			if (str == "OnDestroy")                return ManagedMethod::OnDestroy;
			if (str == "OnCollisionEnter")         return ManagedMethod::OnCollisionEnter;
			if (str == "OnCollisionExit")          return ManagedMethod::OnCollisionExit;
			if (str == "OnTriggerEnter")           return ManagedMethod::OnTriggerEnter;
			if (str == "OnTriggerExit")            return ManagedMethod::OnTriggerExit;
			if (str == "OnFixedJointDisconnected") return ManagedMethod::OnFixedJointDisconnected;
			if (str == "OnEnable")                 return ManagedMethod::OnEnable;
			if (str == "OnDisable")                return ManagedMethod::OnDisable;
			if (str == "OnGui")                    return ManagedMethod::OnGui;

			VX_CORE_ASSERT(false, "Unknown Managed Method!");
			return (ManagedMethod)0;
		}

		std::string StringFromManagedMethod(ManagedMethod method)
		{
			switch (method)
			{
				case ManagedMethod::OnAwake:                  return "OnAwake";
				case ManagedMethod::OnCreate:                 return "OnCreate";
				case ManagedMethod::OnUpdateDelta:            return "OnUpdateDelta";
				case ManagedMethod::OnUpdate:                 return "OnUpdate";
				case ManagedMethod::OnDestroy:                return "OnDestroy";
				case ManagedMethod::OnCollisionEnter:         return "OnCollisionEnter";
				case ManagedMethod::OnCollisionExit:          return "OnCollisionExit";
				case ManagedMethod::OnTriggerEnter:           return "OnTriggerEnter";
				case ManagedMethod::OnTriggerExit:            return "OnTriggerExit";
				case ManagedMethod::OnFixedJointDisconnected: return "OnFixedJointDisconnected";
				case ManagedMethod::OnEnable:                 return "OnEnabled";
				case ManagedMethod::OnDisable:                return "OnDisabled";
				case ManagedMethod::OnGui:                    return "OnGui";
			}

			VX_CORE_ASSERT(false, "Unknown Managed Method!");
			return "";
		}

	}

}
