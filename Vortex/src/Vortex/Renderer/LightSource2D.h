#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex {

	struct VORTEX_API LightSource2DProperties
	{
		Math::vec3 Color = Math::vec3(1.0f);
		float Intensity = 1.0f;
	};
	
	class VORTEX_API LightSource2D
	{
	public:
		LightSource2D() = default;
		LightSource2D(const LightSource2DProperties& props);
		~LightSource2D() = default;
		
		const Math::vec3& GetColor() const;
		void SetColor(const Math::vec3& color);

		float GetIntensity() const;
		void SetIntensity(float intensity);

		static void Copy(SharedRef<LightSource2D>& dest, const SharedRef<LightSource2D>& src);

		static SharedRef<LightSource2D> Create(const LightSource2DProperties& props);

	private:
		LightSource2DProperties m_Properties;
	};

}
