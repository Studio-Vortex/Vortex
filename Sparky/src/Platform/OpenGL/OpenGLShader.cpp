#include "sppch.h"
#include "OpenGLShader.h"

#include <Glad/glad.h>

namespace Sparky {

	OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		const GLchar* source = (const GLchar*)vertexSrc.c_str();
		glShaderSource(vertexShader, 1, &source, 0);

		glCompileShader(vertexShader);

		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(vertexShader);

			SP_CORE_ERROR("{}", &infoLog[0]);
			SP_CORE_ASSERT(false, "Vertex shader compilation failed!");
			return;
		}

		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		source = (const GLchar*)fragmentSrc.c_str();
		glShaderSource(fragmentShader, 1, &source, 0);

		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(fragmentShader);
			glDeleteShader(vertexShader);

			SP_CORE_ERROR("{}", &infoLog[0]);
			SP_CORE_ASSERT(false, "Fragment shader compilation failed!");
			return;
		}

		m_RendererID = glCreateProgram();

		glAttachShader(m_RendererID, vertexShader);
		glAttachShader(m_RendererID, fragmentShader);

		glLinkProgram(m_RendererID);

		GLint isLinked = 0;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(m_RendererID);

			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			SP_CORE_ERROR("{}", &infoLog[0]);
			SP_CORE_ASSERT(false, "Failed to link Shader Program!");
			return;
		}

		glDetachShader(m_RendererID, vertexShader);
		glDetachShader(m_RendererID, fragmentShader);
	}

	OpenGLShader::~OpenGLShader()
	{
		if (m_RendererID)
			glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Enable() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Disable() const
	{
		glUseProgram(NULL);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, int v) const
	{
		glProgramUniform1i(m_RendererID, GetUniformLocation(uniformName), v);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, unsigned int v) const
	{
		glProgramUniform1ui(m_RendererID, GetUniformLocation(uniformName), v);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, float v) const
	{
		glProgramUniform1f(m_RendererID, GetUniformLocation(uniformName), v);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, double v) const
	{
		glProgramUniform1d(m_RendererID, GetUniformLocation(uniformName), v);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, bool v) const
	{
		glProgramUniform1i(m_RendererID, GetUniformLocation(uniformName), (int)v);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, const Math::vec2& vector) const
	{
		glProgramUniform2f(m_RendererID, GetUniformLocation(uniformName), vector.x, vector.y);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, const Math::vec3& vector) const
	{
		glProgramUniform3f(m_RendererID, GetUniformLocation(uniformName), vector.x, vector.y, vector.z);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, const Math::vec4& vector) const
	{
		glProgramUniform4f(m_RendererID, GetUniformLocation(uniformName), vector.x, vector.y, vector.z, vector.w);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, const Math::mat3& matrix) const
	{
		glProgramUniformMatrix3fv(m_RendererID, GetUniformLocation(uniformName), 1, false, Math::ValuePtr(matrix));
	}
	
	void OpenGLShader::SetUniform(const std::string& uniformName, const Math::mat4& matrix) const
	{
		glProgramUniformMatrix4fv(m_RendererID, GetUniformLocation(uniformName), 1, false, Math::ValuePtr(matrix));
	}

	int OpenGLShader::GetUniformLocation(const std::string& uniformName) const
	{
		return glGetUniformLocation(m_RendererID, uniformName.c_str());
	}

}