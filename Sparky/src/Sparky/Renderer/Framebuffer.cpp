#include "sppch.h"
#include "Framebuffer.h"

#include "Sparky/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

/*
*** #ifdef SP_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DFramebuffer.h"
*** #endif // SP_PLATFORM_WINDOWS
*/

namespace Sparky {

	SharedRef<Framebuffer> Framebuffer::Create(const FramebufferProperties& props)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLFramebuffer>(props);
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

}