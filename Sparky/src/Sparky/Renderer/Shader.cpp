#include "sppch.h"
#include "Shader.h"

#include "Sparky/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

/*
*** #ifdef SP_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DShader.h"
*** #endif // SP_PLATFORM_WINDOWS
*/

namespace Sparky {

	SharedRef<Shader> Shader::Create(const std::string& filepath)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLShader>(filepath);
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	SharedRef<Shader> Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return std::make_shared<OpenGLShader>(vertexSrc, fragmentSrc);
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

}