#include "sppch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <Glad/glad.h>

namespace Sparky {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		SP_CORE_ASSERT(windowHandle, "Window Handle is null!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		SP_CORE_ASSERT(status, "Failed to initialize Glad!");

#ifdef SP_DEBUG
		SP_CORE_INFO("OpenGL Info:");
		SP_CORE_INFO("  Hardware:     {}", glGetString(GL_RENDERER));
		SP_CORE_INFO("  Vendor:       {}", glGetString(GL_VENDOR));
		SP_CORE_INFO("  Version:      {}", glGetString(GL_VERSION));

		SP_CORE_INFO("  GLSL Info:");
		SP_CORE_INFO("    Version:    {}", glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif // SP_DEBUG
	}

	void OpenGLContext::SwapFrameBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

}