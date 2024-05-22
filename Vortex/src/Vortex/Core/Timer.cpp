#include "vxpch.h"
#include "Timer.h"

#include "Vortex/Core/String.h"

namespace Vortex {

	Timer::Timer(const std::string& name, float timeLeft, const OnFinishedFn& onFinished)
		: m_TimerName(name), m_TimeLeft(timeLeft), m_OnFinishedCallback(onFinished)
	{
		m_IsStarted = false;
		m_IsFinished = false;
	}

	void Timer::Start()
	{
		m_IsStarted = true;
	}

	void Timer::OnUpdate(TimeStep delta)
	{
		// the timer hasn't yet started, we really shouldn't be calling OnUpdate yet
		if (IsStarted() == false)
		{
			return;
		}

		// we don't want to call the OnFinished callback multiple times
		if (IsFinished())
		{
			return;
		}

		m_TimeLeft -= delta;

		if (GetTimeLeft() <= 0.0f)
		{
			if (m_OnFinishedCallback != nullptr)
			{
				std::invoke(m_OnFinishedCallback);
			}

			m_IsFinished = true;
		}
	}

	float Timer::GetTimeLeft() const
	{
		return m_TimeLeft;
	}

	const std::string& Timer::GetName() const
	{
		return m_TimerName;
	}

	bool Timer::IsStarted() const
	{
		return m_IsStarted;
	}

	bool Timer::IsFinished() const
	{
		return m_IsFinished;
	}

	bool Timer::operator==(const Timer& other) const
	{
		return String::FastCompare(m_TimerName, other.m_TimerName)
			&& m_TimeLeft == other.m_TimeLeft
			&& m_IsStarted == other.m_IsStarted
			&& m_IsFinished == other.m_IsFinished;
	}

}
