#pragma once

#include <unordered_map>

namespace Vortex {

	struct LightSourceProperties
	{
		Math::vec3 Radiance = Math::vec3(1.0f);

		float Intensity = 1.0f;

		// Spotlight
		float Cutoff = 12.5f;
		float OuterCutoff = 17.5f;

		// Shadow Settings
		float ShadowBias = 0.2f;
		bool CastShadows = true;
		bool SoftShadows = true;
	};

	class LightSource
	{
	public:
		LightSource(const LightSourceProperties& props);

		const Math::vec3& GetRadiance() const;
		void SetRadiance(const Math::vec3& radiance);

		float GetIntensity() const;
		void SetIntensity(float intensity);

		float GetCutOff() const;
		void SetCutOff(float cutoff);

		float GetOuterCutOff() const;
		void SetOuterCutOff(float outerCutoff);

		float GetShadowBias() const;
		void SetShadowBias(float bias);

		bool GetCastShadows() const;
		void SetCastShadows(bool castShadows);

		bool GetSoftShadows() const;
		void SetSoftShadows(bool softShadows);

		uint32_t GetPointLightIndex() const;
		void SetPointLightIndex(uint32_t index);

		uint32_t GetSpotLightIndex() const;
		void SetSpotLightIndex(uint32_t index);

		static void Copy(const SharedRef<LightSource>& dstLightSource, const SharedRef<LightSource>& srcLightSource);

		static SharedRef<LightSource> Create(const LightSourceProperties& props);

	private:
		LightSourceProperties m_Properties;
		uint32_t m_PointLightIndex = 0;
		uint32_t m_SpotLightIndex = 0;
	};

}
