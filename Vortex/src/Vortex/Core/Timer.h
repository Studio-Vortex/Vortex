#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/TimeStep.h"

#include <functional>
#include <string>

namespace Vortex {

	class VORTEX_API Timer
	{
	public:
		using OnFinishedFn = std::function<void()>;

	public:
		Timer(const std::string& name, float timeLeft, const OnFinishedFn& onFinished);

		void Start();

		void OnUpdate(TimeStep delta);

		float GetTimeLeft() const;
		const std::string& GetName() const;

		bool IsStarted() const;
		bool IsFinished() const;

		bool operator==(const Timer& other) const;

	private:
		std::string m_TimerName = "";
		float m_TimeLeft = 0.0f;
		
		OnFinishedFn m_OnFinishedCallback = nullptr;

		bool m_IsStarted = false;
		bool m_IsFinished = false;
	};

}
