#include "sppch.h"
#include "OpenGLShader.h"

#include <Glad/glad.h>

#include <fstream>

namespace Sparky {

	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex" || type == "Vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "Fragment" || type == "pixel" || type == "Pixel")
			return GL_FRAGMENT_SHADER;

		SP_CORE_ASSERT(false, "Invalid Shader Type!");
		return NULL;
	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
	{
		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);
	}

	OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(sources);
	}

	OpenGLShader::~OpenGLShader()
	{
		if (m_RendererID)
			glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath) const
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in, std::ios::binary);

		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			SP_CORE_ERROR("Failed to load shader from: '/{}'", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source) const
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			SP_CORE_ASSERT(eol != std::string::npos, "Syntax Error!");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			SP_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type!");

			size_t nextLinePos = source.find_first_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string> shaderSources)
	{
		std::vector<GLuint> glShaderIDs(shaderSources.size());
		GLuint program = glCreateProgram();

		for (auto& [key, value] : shaderSources)
		{
			GLenum type = key;
			const std::string& source = value;

			GLuint shader = glCreateShader(type);

			const GLchar* sourceCstr = (const GLchar*)source.c_str();
			glShaderSource(shader, 1, &sourceCstr, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				SP_CORE_ERROR("{}", &infoLog[0]);
				SP_CORE_ASSERT(false, "Shader compilation failed!");
				break;
			}
			
			glAttachShader(program, shader);

			glShaderIDs.push_back(shader);
		}

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);

		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);

			for (auto id : glShaderIDs)
				glDeleteShader(id);

			SP_CORE_ERROR("{}", &infoLog[0]);
			SP_CORE_ASSERT(false, "Failed to link Shader Program!");
			return;
		}

		for (auto id : glShaderIDs)
			glDetachShader(program, id);

		m_RendererID = program;
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