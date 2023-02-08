#include "vxpch.h"
#include "Shader.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Platform/OpenGL/OpenGLShader.h"

/*
*** #ifdef VX_PLATFORM_WINDOWS
***		#include "Platform/Direct3D/Direct3DShader.h"
*** #endif // VX_PLATFORM_WINDOWS
*/

namespace Vortex {

	SharedRef<Shader> Shader::Create(const std::string& filepath)
	{
		switch (Renderer::GetGraphicsAPI())
		{
			case RendererAPI::API::None:     VX_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
			case RendererAPI::API::OpenGL:   return CreateShared<OpenGLShader>(filepath);
#ifdef VX_PLATFORM_WINDOWS
			case RendererAPI::API::Direct3D: return nullptr;
#endif // VX_PLATFORM_WINDOWS
			case RendererAPI::API::Vulkan:   return nullptr;
		}

		VX_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	SharedRef<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc)
	{
		switch (Renderer::GetGraphicsAPI())
		{
		case RendererAPI::API::None:     VX_CORE_ASSERT(false, "Renderer API was set to RendererAPI::None!"); return nullptr;
		case RendererAPI::API::OpenGL:   return CreateShared<OpenGLShader>(name, vertexSrc, fragmentSrc, geometrySrc);
#ifdef VX_PLATFORM_WINDOWS
		case RendererAPI::API::Direct3D: return nullptr;
#endif // VX_PLATFORM_WINDOWS
		case RendererAPI::API::Vulkan:   return nullptr;
		}

		VX_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}

	void ShaderLibrary::Add(const std::string& name, const SharedRef<Shader>& shader)
	{
		VX_CORE_ASSERT(!Exists(name), "Shader already exists!");
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
		VX_CORE_ASSERT(Exists(name), "Shader not found!")
		return m_Shaders.find(name)->second;
	}

    const SharedRef<Shader>& ShaderLibrary::Get(const std::string& name) const
    {
		VX_CORE_ASSERT(Exists(name), "Shader not found!");
		return m_Shaders.at(name);
    }

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

    size_t ShaderLibrary::Size() const
    {
		return m_Shaders.size();
    }

	SharedRef<ShaderLibrary> ShaderLibrary::Create()
	{
		return CreateShared<ShaderLibrary>();
	}

}