#include "vxpch.h"
#include "LightSource2D.h"

namespace Vortex {

	LightSource2D::LightSource2D(const LightSource2DProperties& props)
		: m_Properties(props) { }

	const Math::vec3& LightSource2D::GetColor() const
	{
		return m_Properties.Color;
	}

	void LightSource2D::SetColor(const Math::vec3& color)
	{
		m_Properties.Color = color;
	}

	float LightSource2D::GetIntensity() const
	{
		return m_Properties.Intensity;
	}

	void LightSource2D::SetIntensity(float intensity)
	{
		m_Properties.Intensity = intensity;
	}

	void LightSource2D::Copy(SharedRef<LightSource2D>& dest, const SharedRef<LightSource2D>& src)
	{
		dest->SetColor(src->GetColor());
		dest->SetIntensity(src->GetIntensity());
	}

	SharedRef<LightSource2D> LightSource2D::Create(const LightSource2DProperties& props)
	{
		return CreateShared<LightSource2D>(props);
	}

}
