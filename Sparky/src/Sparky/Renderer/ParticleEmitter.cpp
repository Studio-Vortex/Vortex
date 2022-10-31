#include "sppch.h"
#include "ParticleEmitter.h"

#include "Sparky/Scene/Entity.h"

namespace Sparky {

	ParticleEmitter::ParticleEmitter(const ParticleEmitterProperties& props)
		: m_Properties(props)
	{
		for (uint32_t i = 0; i < s_MaxParticles; i++)
		{
			Math::vec3 position((float)i, (float)i, 0.0f);
			Math::vec3 velocity((float)i);
			Math::vec4 color((float)i);
			Math::vec2 size(0.5f);
			m_Particles.push_back({ position, velocity, color, size });
		}
	}

	void ParticleEmitter::OnUpdate(TimeStep delta)
	{
		for (auto& particle : m_Particles)
		{
			particle.Position -= particle.Velocity * Math::vec3(delta);
		}
	}

	SharedRef<ParticleEmitter> ParticleEmitter::Create(const ParticleEmitterProperties& props)
	{
		return CreateShared<ParticleEmitter>(props);
	}

}
