#include "sppch.h"
#include "Buffer.h"

#include "Sparky/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

/*
*** #ifdef SP_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DBuffer.h"
*** #endif // SP_PLATFORM_WINDOWS
*/

namespace Sparky {

	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::OpenGL:   return new OpenGLVertexBuffer(vertices, size);
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::Vulkan:   return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::OpenGL:   return new OpenGLIndexBuffer(indices, size);
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::Vulkan:   return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

}