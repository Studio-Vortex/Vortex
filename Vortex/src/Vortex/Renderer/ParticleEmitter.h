#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/TimeStep.h"

namespace Vortex {

	struct ParticleEmitterProperties
	{
		Math::vec3 Position;
		Math::vec3 Offset = Math::vec3(0.0f);
		Math::vec3 Velocity = Math::vec3(1.0f);
		Math::vec3 VelocityVariation = Math::vec3(1.0f);
		Math::vec2 SizeBegin = Math::vec2(1.0f);
		Math::vec2 SizeEnd = Math::vec2(1.0f);
		Math::vec2 SizeVariation = Math::vec2(1.0f);
		Math::vec4 ColorBegin = Math::vec4(1.0f);
		Math::vec4 ColorEnd = Math::vec4(1.0f);
		float Rotation = 0.1f;
		float LifeTime = 1.0f;
	};

	class ParticleEmitter
	{
	public:
		struct Particle
		{
			Math::vec3 Position;
			Math::vec3 Velocity;
			Math::vec2 SizeBegin, SizeEnd;
			Math::vec4 ColorBegin, ColorEnd;
			float Rotation = 0.0f;

			float LifeTime = 1.0f;
			float LifeRemaining = 0.0f;

			bool Active = false;
		};

	public:
		ParticleEmitter(const ParticleEmitterProperties& props);

		const ParticleEmitterProperties& GetProperties() const { return m_Properties; }
		ParticleEmitterProperties& GetProperties() { return m_Properties; }
		void SetProperties(const ParticleEmitterProperties& props);

		const std::vector<Particle>& GetParticles() const { return m_ParticlePool; }

		void Start();
		void Stop();

		void OnUpdate(TimeStep delta);

		void EmitParticle();

		bool IsActive() const { return m_IsActive; }

		static void Copy(const SharedRef<ParticleEmitter>& dstEmitter, const SharedRef<ParticleEmitter>& srcEmitter);

		static SharedRef<ParticleEmitter> Create(const ParticleEmitterProperties& props);

	private:
		inline static constexpr uint32_t s_MaxParticles = 1'000;

	private:
		ParticleEmitterProperties m_Properties;
		std::vector<Particle> m_ParticlePool;
		uint32_t m_PoolIndex = s_MaxParticles - 1;
		bool m_IsActive = false;
	};

}
