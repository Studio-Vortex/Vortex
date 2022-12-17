#include "vxpch.h"
#include "LightSource.h"

namespace Vortex {

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
		dstLightSource->SetDirection(srcLightSource->GetDirection());
		dstLightSource->SetPosition(srcLightSource->GetPosition());
		dstLightSource->SetAttenuation(srcLightSource->GetAttenuation());
		dstLightSource->SetCutOff(srcLightSource->GetCutOff());
		dstLightSource->SetOuterCutOff(srcLightSource->GetOuterCutOff());
	}

    SharedRef<LightSource> LightSource::Create(const LightSourceProperties& props)
    {
		return CreateShared<LightSource>(props);
    }

}
