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

    float LightSource::GetIntensity() const
    {
        return m_Properties.Intensity;
    }

    void LightSource::SetIntensity(float intensity)
    {
		m_Properties.Intensity = intensity;
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

	float LightSource::GetShadowBias() const
	{
		return m_Properties.ShadowBias;
	}

	void LightSource::SetShadowBias(float bias)
	{
		m_Properties.ShadowBias = bias;
	}

    bool LightSource::ShouldCastShadows() const
    {
        return m_Properties.CastShadows;
    }

    void LightSource::SetCastShadows(bool castShadows)
    {
		m_Properties.CastShadows = castShadows;
    }

	void LightSource::Copy(const SharedRef<LightSource>& dstLightSource, const SharedRef<LightSource>& srcLightSource)
	{
		dstLightSource->SetRadiance(srcLightSource->GetRadiance());
		dstLightSource->SetIntensity(srcLightSource->GetIntensity());
		dstLightSource->SetCutOff(srcLightSource->GetCutOff());
		dstLightSource->SetOuterCutOff(srcLightSource->GetOuterCutOff());
		dstLightSource->SetCastShadows(srcLightSource->ShouldCastShadows());
	}

	SharedRef<LightSource> LightSource::Create(const LightSourceProperties& props)
	{
		return CreateShared<LightSource>(props);
	}

}
