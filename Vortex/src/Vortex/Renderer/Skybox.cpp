#include "vxpch.h"
#include "Skybox.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Platform/OpenGL/OpenGLSkybox.h"

/*
*** #ifdef VX_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DSkybox.h"
*** #endif // VX_PLATFORM_WINDOWS
*/

namespace Vortex {

    void Skybox::Copy(SharedReference<Skybox>& dstSkybox, const SharedReference<Skybox>& srcSkybox)
    {
		dstSkybox->LoadFromFilepath(srcSkybox->GetFilepath());
    }

    SharedReference<Skybox> Skybox::Create()
    {
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     VX_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return SharedReference<OpenGLSkybox>::Create();
#ifdef VX_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // VX_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		VX_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
    }

    SharedReference<Skybox> Skybox::Create(const std::string& filepath)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     VX_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return SharedReference<OpenGLSkybox>::Create(filepath);
#ifdef VX_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // VX_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		VX_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

}
