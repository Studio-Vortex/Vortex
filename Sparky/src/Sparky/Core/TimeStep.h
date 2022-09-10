#pragma once

namespace Sparky {

	class TimeStep
	{
	public:
		TimeStep(float time = 0.0f)
			: m_DeltaTime(time)
		{
			
		}

		/// <returns>The time between each frame in seconds</returns>
		inline float GetDeltaTime() const { return m_DeltaTime; }
		/// <returns>The time between each frame in milliseconds</returns>
		inline float GetDeltaTimeMs() const { return m_DeltaTime * 1000; }

		inline operator float() const { return m_DeltaTime; }

		inline float operator+(float other) const
		{
			return m_DeltaTime + other;
		}

		inline void operator+=(float other)
		{
			m_DeltaTime += other;
		}
		
	private:
		float m_DeltaTime;
	};

}