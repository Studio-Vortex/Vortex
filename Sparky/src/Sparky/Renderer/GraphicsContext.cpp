#include "sppch.h"
#include "GraphicsContext.h"

#include "Sparky/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"

/*
*** #ifdef SP_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DContext.h"
*** #endif // SP_PLATFORM_WINDOWS
*/

namespace Sparky {

	UniqueRef<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateUnique<OpenGLContext>(static_cast<GLFWwindow*>(window));
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:  return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

}