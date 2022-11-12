#pragma once

#include "Sparky/Renderer/Material.h"

namespace Sparky {

	class MaterialInstance
	{
	public:
		MaterialInstance(const SharedRef<Material>& material);
		~MaterialInstance() = default;

	private:
		SharedRef<Material> m_BaseMaterial;
	};

}
