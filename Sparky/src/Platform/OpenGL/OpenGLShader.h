#pragma once

#include "Sparky/Renderer/Shader.h"

#include "Sparky/Core/Math.h"

#include <unordered_map>
#include <string>

// TODO: REMOVE!!!
typedef unsigned int GLenum;

namespace Sparky {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		~OpenGLShader() override;

		void Enable() const override;
		void Disable() const override;

		void SetBool(const std::string& name, bool value) const override;
		void SetInt(const std::string& name, int value) const override;
		void SetIntArray(const std::string& name, int* data, uint32_t count) const override;
		void SetFloat(const std::string& name, float value) const override;
		void SetMat3(const std::string& name, const Math::mat3& matrix) const override;
		void SetMat4(const std::string & name, const Math::mat4 & matrix) const override;
		void SetFloat2(const std::string & name, const Math::vec2 & vector) const override;
		void SetFloat3(const std::string & name, const Math::vec3 & vector) const override;
		void SetFloat4(const std::string & name, const Math::vec4 & vector) const override;

		inline const std::string& GetName() const override { return m_Name; };

		void ReCompile() override;

		void SetUniform(const std::string& uniformName, int v) const;
		void SetUniform(const std::string& uniformName, int* data, uint32_t count) const;
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
		void CreateShader(const std::string& filepath);

		std::string ReadFile(const std::string& filepath) const;
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source) const;
		void Compile(const std::unordered_map<GLenum, std::string> shaderSources);
		int GetUniformLocation(const std::string& uniformName) const;

	private:
		uint32_t m_RendererID = 0;
		std::string m_Filepath;
		std::string m_Name;

		mutable std::unordered_map<std::string, int32_t> m_UniformLocationCache;
	};

}
