#pragma once

namespace Vortex {

	struct LightSourceProperties
	{
		Math::vec3 Radiance = Math::vec3(1.0f);

		float Intensity = 1.0f;

		// Spotlight
		float Cutoff = 12.5f;
		float OuterCutoff = 17.5f;

		// Shadow Settings
		float ShadowBias = 5.0f;
		bool CastShadows = true;
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

		bool ShouldCastShadows() const;
		void SetCastShadows(bool castShadows);

		static void Copy(const SharedRef<LightSource>& dstLightSource, const SharedRef<LightSource>& srcLightSource);

		static SharedRef<LightSource> Create(const LightSourceProperties& props);

	private:
		LightSourceProperties m_Properties;
	};

}
