#pragma once

#include "Sparky/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Sparky {

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		void Init() override;
		void SwapFrameBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
	};

}