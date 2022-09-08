#pragma once

#include "Sparky/Renderer/Shader.h"

#include "Sparky/Math.h"

#include <string>

// TODO: REMOVE!!!
typedef unsigned int GLenum;

namespace Sparky {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader();

		void Enable() const override;
		void Disable() const override;

		inline const std::string& GetName() const override { return m_Name; };

		void SetUniform(const std::string& uniformName, int v) const;
		void SetUniform(const std::string& uniformName, unsigned int v) const;
		void SetUniform(const std::string& uniformName, float v) const;
		void SetUniform(const std::string& uniformName, double v) const;
		void SetUniform(const std::string& uniformName, bool v) const;
		void SetUniform(const std::string& uniformName, const Math::vec2& vector) const;
		void SetUniform(const std::string& uniformName, const Math::vec3& vector) const;
		void SetUniform(const std::string& uniformName, const Math::vec4& vector) const;
		void SetUniform(const std::string& uniformName, const Math::mat3& matrix) const;
		void SetUniform(const std::string& uniformName, const Math::mat4& matrix) const;

	private:
		std::string ReadFile(const std::string& filepath) const;
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source) const;
		void Compile(const std::unordered_map<GLenum, std::string> shaderSources);
		int GetUniformLocation(const std::string& uniformName) const;

	private:
		uint32_t m_RendererID = 0;
		std::string m_Name;
	};

}