#pragma once

namespace Vortex {

	enum class ManagedMethod
	{
		OnAwake,
		OnCreate,
		OnUpdateDelta,
		OnUpdate,
		OnDestroy,
		OnCollisionEnter,
		OnCollisionExit,
		OnTriggerEnter,
		OnTriggerExit,
		OnFixedJointDisconnected,
		OnRaycastCollision,
		OnEnabled,
		OnDisabled,
		OnGui,
	};

}
