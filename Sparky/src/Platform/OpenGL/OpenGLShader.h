#pragma once

#include "Sparky/Renderer/Shader.h"

#include "Sparky/Math.h"

#include <string>

namespace Sparky {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader();

		void Enable() const override;
		void Disable() const override;

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
		int GetUniformLocation(const std::string& uniformName) const;

	private:
		uint32_t m_RendererID = 0;
	};

}