#include "vxpch.h"

#include "Vortex/Utils/Time.h"

#include <GLFW/glfw3.h>

namespace Vortex {

	static TimeStep s_DeltaTime = 0.0f;

	float Time::GetTime()
	{
		return glfwGetTime();
	}

	TimeStep Time::GetDeltaTime()
	{
		return s_DeltaTime;
	}

	void Time::SetDeltaTime(TimeStep delta)
	{
		s_DeltaTime = delta;
	}

}
