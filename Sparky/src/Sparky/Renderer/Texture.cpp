#include "sppch.h"
#include "Texture.h"

#include "Sparky/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

/*
*** #ifdef SP_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DTexture.h"
*** #endif // SP_PLATFORM_WINDOWS
*/

namespace Sparky {
    
	SharedRef<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLTexture2D>(width, height);
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
    }

    SharedRef<Texture2D> Texture2D::Create(const std::string& path, bool flipVertical)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLTexture2D>(path, flipVertical);
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
	
}
