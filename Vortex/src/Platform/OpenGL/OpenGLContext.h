#pragma once

#include "Vortex/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Vortex {

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