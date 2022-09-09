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
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLShader>(filepath);
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	SharedRef<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     SP_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLShader>(name, vertexSrc, fragmentSrc);
#ifdef SP_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // SP_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		SP_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	void ShaderLibrary::Add(const std::string& name, const SharedRef<Shader>& shader)
	{
		SP_CORE_ASSERT(!Exists(name), "Shader already exists!");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const SharedRef<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	SharedRef<Shader> ShaderLibrary::Load(const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	SharedRef<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	SharedRef<Shader> ShaderLibrary::Get(const std::string& name)
	{
		SP_CORE_ASSERT(Exists(name), "Shader not found!")
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

}