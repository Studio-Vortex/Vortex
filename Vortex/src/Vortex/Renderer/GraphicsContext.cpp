#include "vxpch.h"
#include "GraphicsContext.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Platform/OpenGL/OpenGLContext.h"

#ifdef VX_PLATFORM_WINDOWS
	#include "Vortex/Platform/DirectX/DirectXContext.h"
#endif

namespace Vortex {

	UniqueRef<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     VX_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateUnique<OpenGLContext>(static_cast<GLFWwindow*>(window));
#ifdef VX_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return CreateUnique<DirectXContext>(static_cast<GLFWwindow*>(window));
#endif // VX_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:  return nullptr;
		}

		VX_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

}