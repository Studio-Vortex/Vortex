#include "vxpch.h"
#include "Texture.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Platform/OpenGL/OpenGLTexture.h"

/*
*** #ifdef VX_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DTexture.h"
*** #endif // VX_PLATFORM_WINDOWS
*/

namespace Vortex {

    SharedRef<Texture2D> Texture2D::Create(const ImageProperties& imageProps)
    {
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     VX_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLTexture2D>(imageProps);
#ifdef VX_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // VX_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		VX_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
    }
	
}
