#include "sppch.h"
#include "LightSource.h"

namespace Sparky {

	LightSource::LightSource(const LightSourceProperties& props)
		: m_Properties(props) { }

	const Math::vec3& LightSource::GetAmbient() const
	{
		return m_Properties.Ambient;
	}

	void LightSource::SetAmbient(const Math::vec3& ambient)
	{
		m_Properties.Ambient = ambient;
	}

	const Math::vec3& LightSource::GetDiffuse() const
	{
		return m_Properties.Diffuse;
	}

	void LightSource::SetDiffuse(const Math::vec3& diffuse)
	{
		m_Properties.Diffuse = diffuse;
	}

	const Math::vec3& LightSource::GetSpecular() const
	{
		return m_Properties.Specular;
	}

	void LightSource::SetSpecular(const Math::vec3& specular)
	{
		m_Properties.Specular = specular;
	}

    const Math::vec3& LightSource::GetColor() const
    {
		return m_Properties.Color;
    }

    void LightSource::SetColor(const Math::vec3& color)
    {
		m_Properties.Color = color;
    }

    const Math::vec3& LightSource::GetPosition() const
    {
		return  m_Properties.Position;
    }

    void LightSource::SetPosition(const Math::vec3& position)
    {
		m_Properties.Position = position;
    }

    SharedRef<LightSource> LightSource::Create(const LightSourceProperties& props)
    {
		return CreateShared<LightSource>(props);
    }

}
