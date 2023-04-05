#include "vxpch.h"
#include "DirectXSwapchain.h"

namespace Vortex {

	void DirectXSwapchain::Present()
	{
		// TODO make these configurable
		DX_CHECK(m_Swapchain->Present(1, 0), "Failed to Present Back Buffer!");
	}

}
