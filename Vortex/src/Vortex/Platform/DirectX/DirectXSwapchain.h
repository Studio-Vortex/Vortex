#pragma once

#include "Vortex/Platform/DirectX/DirectX.h"

namespace Vortex {

	class DirectXSwapchain
	{
	public:
		DirectXSwapchain() = default;
		~DirectXSwapchain() = default;

		VX_FORCE_INLINE IDXGISwapChain** GetAddressOf() { return m_Swapchain.GetAddressOf(); }
		VX_FORCE_INLINE IDXGISwapChain* const* GetAddressOf() const { return m_Swapchain.GetAddressOf(); }
		VX_FORCE_INLINE IDXGISwapChain* Get() const { return m_Swapchain.Get(); }

		void Present();

	private:
		ComPtr<IDXGISwapChain> m_Swapchain = nullptr;
	};

}
