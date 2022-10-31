#pragma once

namespace Sparky {

	struct LightSourceProperties
	{
		Math::vec3 Ambient = Math::vec3(0.2f);
		Math::vec3 Diffuse = Math::vec3(0.5f);
		Math::vec3 Specular = Math::vec3(0.5f);

		Math::vec3 Color = Math::vec3(1.0f);
		Math::vec3 Position = Math::vec3(0.0f);
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

		static SharedRef<LightSource> Create(const LightSourceProperties& props);

	private:
		LightSourceProperties m_Properties;
	};

}
