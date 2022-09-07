#include "sppch.h"
#include "VertexArray.h"

#include "Sparky/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

/*
*** #ifdef SP_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DVertexArray.h"
*** #endif // SP_PLATFORM_WINDOWS
*/

namespace Sparky {

	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetGraphicsAPI())
		{
		case RendererAPI::API::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
		case RendererAPI::API::OpenGL:   return new OpenGLVertexArray();
#ifdef SP_PLATFORM_WINDOWS
		case RendererAPI::API::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
		case RendererAPI::API::Vulkan :  return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

}