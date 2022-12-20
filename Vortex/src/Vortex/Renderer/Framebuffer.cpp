#include "vxpch.h"
#include "Framebuffer.h"

#include "Vortex/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

/*
*** #ifdef VX_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DFramebuffer.h"
*** #endif // VX_PLATFORM_WINDOWS
*/

namespace Vortex {

	SharedRef<Framebuffer> Framebuffer::Create(const FramebufferProperties& props)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     VX_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLFramebuffer>(props);
#ifdef VX_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // VX_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		VX_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
	
	SharedRef<HDRFramebuffer> HDRFramebuffer::Create(const FramebufferProperties& props)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     VX_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLHDRFramebuffer>(props);
#ifdef VX_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // VX_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		VX_CORE_ASSERT(false, "Unknown Renderer API!");
	}

    SharedRef<DepthMapFramebuffer> DepthMapFramebuffer::Create(const FramebufferProperties& props)
    {
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     VX_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLDepthMapFramebuffer>(props);
#ifdef VX_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // VX_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		VX_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
    }

}
