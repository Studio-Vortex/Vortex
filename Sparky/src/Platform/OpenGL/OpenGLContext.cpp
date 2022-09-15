#include "sppch.h"
#include "OpenGLContext.h"

#include "Sparky/Renderer/Renderer.h"

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
		SP_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		SP_CORE_ASSERT(status, "Failed to initialize Glad!");

		Renderer::SetGraphicsAPIInfo({
			"OpenGL",
			glGetString(GL_RENDERER),
			glGetString(GL_VENDOR),
			glGetString(GL_VERSION),
			glGetString(GL_SHADING_LANGUAGE_VERSION),
		});

#ifdef SP_DEBUG
		SP_CORE_INFO("OpenGL Info:");
		SP_CORE_INFO("  Hardware:     {}", glGetString(GL_RENDERER));
		SP_CORE_INFO("  Vendor:       {}", glGetString(GL_VENDOR));
		SP_CORE_INFO("  Version:      {}", glGetString(GL_VERSION));
		SP_CORE_INFO("  GLSL Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));

		int major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		SP_CORE_ASSERT(major >= 4 && minor >= 5, "Sparky requires OpenGL Version 4.5 or above!");
#endif // SP_DEBUG
	}

	void OpenGLContext::SwapFrameBuffers()
	{
		SP_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}