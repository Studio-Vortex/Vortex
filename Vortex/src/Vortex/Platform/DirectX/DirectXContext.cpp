#include "vxpch.h"
#include "DirectXContext.h"

#include "Vortex/Platform/DirectX/DirectX.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <d3d11.h>

namespace Vortex {

	DirectXContext::DirectXContext(GLFWwindow* window)
	{
		m_Window = glfwGetWin32Window(window);
		glfwGetWindowSize(window, &m_WindowWidth, &m_WindowHeight);
	}

	void DirectXContext::Init()
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
		swapChainDesc.BufferCount = 1u;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.Width = (uint32_t)m_WindowWidth;
		swapChainDesc.BufferDesc.Height = (uint32_t)m_WindowHeight;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1u;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 120u;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Flags = 0;
		swapChainDesc.OutputWindow = m_Window;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		int displayWidth = GetSystemMetrics(SM_CXSCREEN);
		int displayHeight = GetSystemMetrics(SM_CYSCREEN);
		swapChainDesc.Windowed = m_WindowWidth < displayWidth&& m_WindowHeight < displayHeight;

		DX_CHECK(D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, NULL, nullptr, NULL,
			D3D11_SDK_VERSION, &swapChainDesc, m_Swapchain.GetAddressOf(),
			m_Device->GetAddressOf(), NULL, m_DeviceContext->GetAddressOf()
		), "Failed to Create Device and Swapchain!");

		VX_CORE_VERIFY(m_Swapchain.Get());
	}

	void DirectXContext::SwapFrameBuffers()
	{
		m_Swapchain.Present();
	}

}
