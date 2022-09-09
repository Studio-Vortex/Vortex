#pragma once

#include "Sparky/Core/Core.h"

#include "Sparky/Core/Math.h"

#include <unordered_map>
#include <string>

namespace Sparky {

	class SPARKY_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Enable() const = 0;
		virtual void Disable() const = 0;

		virtual void SetInt(const std::string& name, int value) const = 0;
		virtual void SetFloat(const std::string& name, int value) const = 0;
		virtual void SetMat3(const std::string& name, const Math::mat3& matrix) const = 0;
		virtual void SetMat4(const std::string& name, const Math::mat4& matrix) const = 0;
		virtual void SetFloat2(const std::string& name, const Math::vec2& vector) const = 0;
		virtual void SetFloat3(const std::string& name, const Math::vec3& vector) const = 0;
		virtual void SetFloat4(const std::string& name, const Math::vec4& vector) const = 0;

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