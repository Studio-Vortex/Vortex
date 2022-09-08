#pragma once

#include "Sparky/Core/Core.h"

#include <unordered_map>
#include <string>

namespace Sparky {

	class SPARKY_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Enable() const = 0;
		virtual void Disable() const = 0;

		virtual const std::string& GetName() const = 0;

		static SharedRef<Shader> Create(const std::string& filepath);
		static SharedRef<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};


	class SPARKY_API ShaderLibrary
	{
	public:
		void Add(const std::string& name, const SharedRef<Shader>& shader);
		void Add(const SharedRef<Shader>& shader);
		SharedRef<Shader> Load(const std::string& filepath);
		SharedRef<Shader> Load(const std::string& name, const std::string& filepath);

		SharedRef<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;

	private:
		std::unordered_map<std::string, SharedRef<Shader>> m_Shaders;
	};
}