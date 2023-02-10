#include "vxpch.h"
#include "OpenGLContext.h"

#include "Vortex/Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <Glad/glad.h>

namespace Vortex {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		VX_CORE_ASSERT(windowHandle, "Window Handle is null!");
	}

	void OpenGLContext::Init()
	{
		VX_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		VX_CORE_ASSERT(status, "Failed to initialize Glad!");

		Renderer::SetGraphicsAPIInfo({
			"OpenGL",
			glGetString(GL_RENDERER),
			glGetString(GL_VENDOR),
			glGetString(GL_VERSION),
			glGetString(GL_SHADING_LANGUAGE_VERSION),
		});

#ifdef VX_DEBUG
		VX_CORE_INFO_TAG("RendererAPI", "OpenGL Info:");
		VX_CORE_INFO_TAG("RendererAPI", "  Hardware:     {}", glGetString(GL_RENDERER));
		VX_CORE_INFO_TAG("RendererAPI", "  Vendor:       {}", glGetString(GL_VENDOR));
		VX_CORE_INFO_TAG("RendererAPI", "  Version:      {}", glGetString(GL_VERSION));
		VX_CORE_INFO_TAG("RendererAPI", "  GLSL Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));

		int major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		VX_CORE_ASSERT(major >= 4 && minor >= 5, "Vortex requires OpenGL Version 4.5 or above!");
#endif // VX_DEBUG
	}

	void OpenGLContext::SwapFrameBuffers()
	{
		VX_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}