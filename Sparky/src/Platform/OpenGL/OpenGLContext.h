#pragma once

#include "Sparky/Renderer/RendererContext.h"

struct GLFWwindow;

namespace Sparky {

	class OpenGLContext : public RendererContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		void Init() override;
		void SwapFrameBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
	};

}