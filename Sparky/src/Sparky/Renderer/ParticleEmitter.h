#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Core/TimeStep.h"

namespace Sparky {

	struct ParticleEmitterProperties
	{
		enum class PrimitiveType { Quad = 0, Circle };
		PrimitiveType Type = PrimitiveType::Quad;
	};

	class ParticleEmitter
	{
	public:
		struct Particle
		{
			Math::vec3 Position;
			Math::vec3 Velocity;
			Math::vec4 Color;
			Math::vec2 Size;

			float Rotation = 0.0f;
			float Lifetime = 1.0f;
		};

	public:
		ParticleEmitter(const ParticleEmitterProperties& props);

		const ParticleEmitterProperties& GetProperties() const { return m_Properties; }
		ParticleEmitterProperties& GetProperties() { return m_Properties; }

		const std::vector<Particle>& GetParticles() const { return m_Particles; }

		void OnUpdate(TimeStep delta);

		bool IsActive() const { return m_IsActive; }

		void Start() { m_IsActive = true; }
		void Stop() { m_IsActive = false; }

		static SharedRef<ParticleEmitter> Create(const ParticleEmitterProperties& props);

	private:
		inline static constexpr uint32_t s_MaxParticles = 1'000;

	private:
		ParticleEmitterProperties m_Properties;
		std::vector<Particle> m_Particles;
		bool m_IsActive = false;
	};

}
