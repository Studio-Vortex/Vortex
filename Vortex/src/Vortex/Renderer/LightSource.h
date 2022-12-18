#pragma once

namespace Vortex {

	struct LightSourceProperties
	{
		Math::vec3 Radiance = Math::vec3(1.0f);

		// (Linear, Quadratic)
		Math::vec2 Attenuation = Math::vec2(0.045f, 0.0075f);

		float Cutoff = 12.5f;
		float OuterCutoff = 17.5f;
	};

	class LightSource
	{
	public:
		LightSource(const LightSourceProperties& props);

		const Math::vec3& GetRadiance() const;
		void SetRadiance(const Math::vec3& radiance);

		const Math::vec2& GetAttenuation() const;
		void SetAttenuation(const Math::vec2& attenuation);

		float GetCutOff() const;
		void SetCutOff(float cutoff);

		float GetOuterCutOff() const;
		void SetOuterCutOff(float outerCutoff);

		static void Copy(const SharedRef<LightSource>& dstLightSource, const SharedRef<LightSource>& srcLightSource);

		static SharedRef<LightSource> Create(const LightSourceProperties& props);

	private:
		LightSourceProperties m_Properties;
	};

}
