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

    bool LightSource::GetCastShadows() const
    {
        return m_Properties.CastShadows;
    }

    void LightSource::SetCastShadows(bool castShadows)
    {
		m_Properties.CastShadows = castShadows;
    }

	bool LightSource::GetSoftShadows() const
	{
		return m_Properties.SoftShadows;
	}

	void LightSource::SetSoftShadows(bool softShadows)
	{
		m_Properties.SoftShadows = softShadows;
	}

	uint32_t LightSource::GetPointLightIndex() const
	{
		return m_PointLightIndex;
	}

	void LightSource::SetPointLightIndex(uint32_t index)
	{
		m_PointLightIndex = index;
	}

	uint32_t LightSource::GetSpotLightIndex() const
	{
		return m_SpotLightIndex;
	}

	void LightSource::SetSpotLightIndex(uint32_t index)
	{
		m_SpotLightIndex = index;
	}

	void LightSource::Copy(const SharedRef<LightSource>& dstLightSource, const SharedRef<LightSource>& srcLightSource)
	{
		dstLightSource->SetRadiance(srcLightSource->GetRadiance());
		dstLightSource->SetIntensity(srcLightSource->GetIntensity());
		dstLightSource->SetCutOff(srcLightSource->GetCutOff());
		dstLightSource->SetOuterCutOff(srcLightSource->GetOuterCutOff());
		dstLightSource->SetCastShadows(srcLightSource->GetCastShadows());
		dstLightSource->SetSoftShadows(srcLightSource->GetSoftShadows());
		dstLightSource->SetPointLightIndex(srcLightSource->GetPointLightIndex());
		dstLightSource->SetSpotLightIndex(srcLightSource->GetSpotLightIndex());
	}

	SharedRef<LightSource> LightSource::Create(const LightSourceProperties& props)
	{
		return CreateShared<LightSource>(props);
	}

}
