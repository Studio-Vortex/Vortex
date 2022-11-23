#include "sppch.h"
#include "LightSource.h"

namespace Sparky {

	LightSource::LightSource(const LightSourceProperties& props)
		: m_Properties(props) { }

    const Math::vec3& LightSource::GetRadiance() const
    {
		return m_Properties.Radiance;
    }

    void LightSource::SetRadiance(const Math::vec3& radiance)
    {
		m_Properties.Radiance = radiance;
    }

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

    const Math::vec3& LightSource::GetDirection() const
    {
		return m_Properties.Direction;
    }

    void LightSource::SetDirection(const Math::vec3& direction)
    {
		m_Properties.Direction = direction;
    }

    const Math::vec2& LightSource::GetAttenuation() const
    {
		return m_Properties.Attenuation;
    }

    void LightSource::SetAttenuation(const Math::vec2& attenuation)
    {
		m_Properties.Attenuation = attenuation;
    }

    float LightSource::GetCutOff() const
    {
		return m_Properties.Cutoff;
    }

    void LightSource::SetCutOff(float cutoff)
    {
		m_Properties.Cutoff = cutoff;
    }

	float LightSource::GetOuterCutOff() const
	{
		return m_Properties.OuterCutoff;
	}

	void LightSource::SetOuterCutOff(float outerCutoff)
	{
		m_Properties.OuterCutoff = outerCutoff;
	}

	void LightSource::Copy(const SharedRef<LightSource>& dstLightSource, const SharedRef<LightSource>& srcLightSource)
	{
		dstLightSource->SetRadiance(srcLightSource->GetRadiance());
		dstLightSource->SetAmbient(srcLightSource->GetAmbient());
		dstLightSource->SetAttenuation(srcLightSource->GetAttenuation());
		dstLightSource->SetColor(srcLightSource->GetColor());
		dstLightSource->SetCutOff(srcLightSource->GetCutOff());
		dstLightSource->SetOuterCutOff(srcLightSource->GetOuterCutOff());
		dstLightSource->SetDiffuse(srcLightSource->GetDiffuse());
		dstLightSource->SetDirection(srcLightSource->GetDirection());
		dstLightSource->SetPosition(srcLightSource->GetPosition());
		dstLightSource->SetSpecular(srcLightSource->GetSpecular());
	}

    SharedRef<LightSource> LightSource::Create(const LightSourceProperties& props)
    {
		return CreateShared<LightSource>(props);
    }

}
