#pragma once

namespace Sparky {

	struct LightSourceProperties
	{
		Math::vec3 Ambient = Math::vec3(0.2f);
		Math::vec3 Diffuse = Math::vec3(0.5f);
		Math::vec3 Specular = Math::vec3(0.5f);

		Math::vec3 Color = Math::vec3(1.0f);
		Math::vec3 Position = Math::vec3(0.0f);
		Math::vec3 Direction = Math::vec3(0.0f);

		// (Linear, Quadratic)
		Math::vec2 Attenuation = Math::vec2(0.045f, 0.0075f);

		float Cutoff = 12.5f;
		float OuterCutoff = 17.5f;
	};

	class LightSource
	{
	public:
		LightSource(const LightSourceProperties& props);

		const Math::vec3& GetAmbient() const;
		void SetAmbient(const Math::vec3& ambient);

		const Math::vec3& GetDiffuse() const;
		void SetDiffuse(const Math::vec3& diffuse);

		const Math::vec3& GetSpecular() const;
		void SetSpecular(const Math::vec3& specular);

		const Math::vec3& GetColor() const;
		void SetColor(const Math::vec3& color);

		const Math::vec3& GetPosition() const;
		void SetPosition(const Math::vec3& position);

		const Math::vec3& GetDirection() const;
		void SetDirection(const Math::vec3& direction);

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
