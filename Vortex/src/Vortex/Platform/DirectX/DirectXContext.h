#pragma once

#include "Vortex/Renderer/GraphicsContext.h"

#include "Vortex/Platform/DirectX/DirectX.h"

#include "Vortex/Platform/DirectX/DirectXDevice.h"
#include "Vortex/Platform/DirectX/DirectXDeviceContext.h"
#include "Vortex/Platform/DirectX/DirectXSwapchain.h"

struct GLFWwindow;

namespace Vortex {

	class DirectXContext : public GraphicsContext
	{
	public:
		DirectXContext(GLFWwindow* window);
		~DirectXContext() override = default;

		void Init() override;
		void SwapFrameBuffers() override;

	private:
		HWND m_Window = nullptr;
		int m_WindowWidth = 0;
		int m_WindowHeight = 0;

		SharedRef<DirectXDevice> m_Device = nullptr;
		SharedRef<DirectXDeviceContext> m_DeviceContext = nullptr;
		DirectXSwapchain m_Swapchain;
	};

}
