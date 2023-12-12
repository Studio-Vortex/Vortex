#pragma once

#include "Vortex/Asset/Asset.h"

#include "Vortex/Renderer/ParticleSystem/Particle.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitterProperties.h"

#include "Vortex/Core/TimeStep.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

#include <string>

namespace Vortex {

	class VORTEX_API ParticleEmitter : public Asset
	{
	public:
		ParticleEmitter() = default;
		ParticleEmitter(const ParticleEmitterProperties& props);
		ParticleEmitter(const ParticleEmitter& other);
		~ParticleEmitter() override = default;

		inline const std::string& GetName() const { return m_Name; }
		inline void SetName(const std::string& name) { m_Name = name; }

		const ParticleEmitterProperties& GetProperties() const { return m_Properties; }
		ParticleEmitterProperties& GetProperties() { return m_Properties; }
		void SetProperties(const ParticleEmitterProperties& props);

		const std::vector<Particle>& GetParticles() const { return m_ParticlePool; }

		void Start();
		void Stop();

		void OnUpdate(TimeStep delta);

		void EmitParticle();

		inline bool IsActive() const { return m_IsActive; };

		ASSET_CLASS_TYPE(ParticleAsset)

		static SharedReference<ParticleEmitter> Create(const ParticleEmitterProperties& props);

	private:
		inline static constexpr uint32_t s_MaxParticles = 1'000;

	private:
		ParticleEmitterProperties m_Properties;
		std::string m_Name;
		
		std::vector<Particle> m_ParticlePool;
		uint32_t m_PoolIndex = s_MaxParticles - 1;

		bool m_IsActive = false;
	};

}
