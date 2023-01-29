#pragma once

#include "Vortex/Renderer/Model.h"

namespace Vortex {

	class MeshFactory
	{
	public:
		static AssetHandle CreateCube(const Math::vec3& size);
		static AssetHandle CreateSphere(float radius);
		static AssetHandle CreateCapsule(float radius, float height);
	};

}
