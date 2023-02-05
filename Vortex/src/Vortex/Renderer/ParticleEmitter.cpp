#include "vxpch.h"
#include "ParticleEmitter.h"

#include "Vortex/Scene/Entity.h"
#include "Vortex/Utils/PlatformUtils.h"

namespace Vortex {

	ParticleEmitter::ParticleEmitter(const ParticleEmitterProperties& props)
		: m_Properties(props)
	{
		m_ParticlePool.resize(s_MaxParticles);
	}

	void ParticleEmitter::Start()
	{
		m_IsActive = true;
	}

	void ParticleEmitter::Stop()
	{
		m_IsActive = false;
	}

	void ParticleEmitter::OnUpdate(TimeStep delta)
	{
		for (auto& particle : m_ParticlePool)
		{
			if (!particle.Active)
				continue;

			if (particle.LifeRemaining <= 0.0f)
			{
				particle.Active = false;
				continue;
			}

			particle.LifeRemaining -= delta;
			particle.Position += particle.Velocity * (float)delta;
			particle.Rotation += m_Properties.Rotation * delta;
		}
	}

	void ParticleEmitter::EmitParticle()
	{
		Particle& particle = m_ParticlePool[m_PoolIndex];
		particle.Active = true;
		particle.Position = m_Properties.Position + m_Properties.Offset;

		if (m_Properties.Rotation != 0.0f)
			particle.Rotation = Random::Float() * 2.0f * Math::PI;
		else
			particle.Rotation = 0.0f;

		// Velocity
		particle.Velocity = m_Properties.Velocity;
		particle.Velocity.x += m_Properties.VelocityVariation.x * (Random::Float() - 0.5f);
		particle.Velocity.y += m_Properties.VelocityVariation.y * (Random::Float() - 0.5f);
		particle.Velocity.z += m_Properties.VelocityVariation.z * (Random::Float() - 0.5f);

		// Color
		particle.ColorBegin = m_Properties.ColorBegin;
		particle.ColorEnd = m_Properties.ColorEnd;

		particle.LifeTime = m_Properties.LifeTime;
		particle.LifeRemaining = m_Properties.LifeTime;
		particle.SizeBegin = m_Properties.SizeBegin + m_Properties.SizeVariation * (Random::Float() - 0.5f);
		particle.SizeEnd = m_Properties.SizeEnd;

		m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
	}

	void ParticleEmitter::Copy(SharedRef<ParticleEmitter> dstEmitter, const SharedRef<ParticleEmitter>& srcEmitter)
	{
		const auto& props = srcEmitter->GetProperties();
		dstEmitter->SetProperties(props);
	}

	void ParticleEmitter::SetProperties(const ParticleEmitterProperties& props)
	{
		m_Properties = props;
	}

	SharedRef<ParticleEmitter> ParticleEmitter::Create(const ParticleEmitterProperties& props)
	{
		return CreateShared<ParticleEmitter>(props);
	}

}
