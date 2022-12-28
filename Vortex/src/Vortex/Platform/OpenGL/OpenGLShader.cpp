#include "vxpch.h"
#include "OpenGLShader.h"

#include "Vortex/Utils/PlatformUtils.h"

#include <Glad/glad.h>

namespace Vortex {

	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex" || type == "Vertex")
			return GL_VERTEX_SHADER;
		if (type == "geometry" || type == "Geometry")
			return GL_GEOMETRY_SHADER;
		if (type == "fragment" || type == "Fragment" || type == "pixel" || type == "Pixel")
			return GL_FRAGMENT_SHADER;

		VX_CORE_ASSERT(false, "Invalid Shader Type!");
		return NULL;
	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
		: m_Filepath(filepath)
	{
		VX_PROFILE_FUNCTION();

		CreateShader(m_Filepath);
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc = std::string())
		: m_Name(name)
	{
		VX_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(sources);
		VX_CORE_INFO("Shader Loaded: {}", m_Name);
	}

	OpenGLShader::~OpenGLShader()
	{
		VX_PROFILE_FUNCTION();

		if (m_RendererID)
			glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::CreateShader(const std::string& filepath)
	{
		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);

		// Extract name from filepath
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
		VX_CORE_INFO("Shader Loaded: '{}' - path /{}", m_Name, filepath);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath) const
	{
		VX_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
				in.close();
			}
			else
			{
				VX_CORE_ERROR("Could not read from file '/{}'", filepath);
			}
		}
		else
		{
			VX_CORE_ERROR("Failed to open shader file from: '/{}'", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source) const
	{
		VX_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			VX_CORE_ASSERT(eol != std::string::npos, "Syntax Error!");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			VX_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type!");

			size_t nextLinePos = source.find_first_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string> shaderSources)
	{
		VX_PROFILE_FUNCTION();

		GLuint program = glCreateProgram();
		VX_CORE_ASSERT(shaderSources.size() <= 3, "Shader Limit Reached!");
		std::array<GLuint, 3> glShaderIDs;
		int glShaderIDIndex = 0;

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

				VX_CORE_ERROR("{}", &infoLog[0]);
				VX_CORE_ASSERT(false, "Shader compilation failed!");
				break;
			}
			
			glAttachShader(program, shader);

			glShaderIDs[glShaderIDIndex++] = shader;
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

			VX_CORE_ERROR("{}", &infoLog[0]);
			VX_CORE_ASSERT(false, "Failed to link Shader Program!");
			return;
		}

		for (auto id : glShaderIDs)
			glDetachShader(program, id);

		m_RendererID = program;
	}

	void OpenGLShader::Enable() const
	{
		VX_PROFILE_FUNCTION();

		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Disable() const
	{
		VX_PROFILE_FUNCTION();

		glUseProgram(NULL);
	}

	void OpenGLShader::Reload()
	{
		if (m_RendererID)
			glDeleteProgram(m_RendererID);

		CreateShader(m_Filepath);
	}

	void OpenGLShader::SetBool(const std::string& name, bool value) const
	{
		VX_PROFILE_FUNCTION();

		SetUniform(name, value);
	}

	void OpenGLShader::SetInt(const std::string& name, int value) const
	{
		VX_PROFILE_FUNCTION();

		SetUniform(name, value);
	}

    void OpenGLShader::SetInt2(const std::string& name, const Math::ivec2& vector) const
    {
		VX_PROFILE_FUNCTION();

		SetUniform(name, vector);
    }

    void OpenGLShader::SetInt3(const std::string& name, const Math::ivec3& vector) const
    {
		VX_PROFILE_FUNCTION();

		SetUniform(name, vector);
    }

    void OpenGLShader::SetInt4(const std::string& name, const Math::ivec4& vector) const
    {
		VX_PROFILE_FUNCTION();

		SetUniform(name, vector);
    }

	void OpenGLShader::SetIntArray(const std::string& name, int* data, uint32_t count) const
	{
		SetUniform(name, data, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value) const
	{
		VX_PROFILE_FUNCTION();

		SetUniform(name, value);
	}

	void OpenGLShader::SetMat3(const std::string& name, const Math::mat3& matrix) const
	{
		VX_PROFILE_FUNCTION();

		SetUniform(name, matrix);
	}

	void OpenGLShader::SetMat4(const std::string& name, const Math::mat4& matrix) const
	{
		VX_PROFILE_FUNCTION();

		SetUniform(name, matrix);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const Math::vec2& vector) const
	{
		VX_PROFILE_FUNCTION();

		SetUniform(name, vector);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const Math::vec3& vector) const
	{
		VX_PROFILE_FUNCTION();

		SetUniform(name, vector);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const Math::vec4& vector) const
	{
		VX_PROFILE_FUNCTION();

		SetUniform(name, vector);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, int v) const
	{
		glProgramUniform1i(m_RendererID, GetUniformLocation(uniformName), v);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, const Math::ivec2& vector) const
	{
		glProgramUniform2i(m_RendererID, GetUniformLocation(uniformName), vector.x, vector.y);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, const Math::ivec3& vector) const
	{
		glProgramUniform3i(m_RendererID, GetUniformLocation(uniformName), vector.x, vector.y, vector.z);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, const Math::ivec4& vector) const
	{
		glProgramUniform4i(m_RendererID, GetUniformLocation(uniformName), vector.x, vector.y, vector.z, vector.w);
	}

	void OpenGLShader::SetUniform(const std::string& uniformName, int* data, uint32_t count) const
	{
		glProgramUniform1iv(m_RendererID, GetUniformLocation(uniformName), count, data);
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
		auto it = m_UniformLocationCache.find(uniformName);
		if (it != m_UniformLocationCache.end())
			return it->second;

		int32_t location = glGetUniformLocation(m_RendererID, uniformName.c_str());
		m_UniformLocationCache[uniformName] = location;
		return location;
	}

}
