#include "sppch.h"
#include "OpenGLVertexArray.h"

#include <Glad/glad.h>

namespace Sparky {

	static GLenum ShaderDataTypeToOpenGLDataType(const ShaderDataType& type)
	{
		switch (type)
		{
			case ShaderDataType::Float:    return GL_FLOAT;
			case ShaderDataType::Float2:   return GL_FLOAT;
			case ShaderDataType::Float3:   return GL_FLOAT;
			case ShaderDataType::Float4:   return GL_FLOAT;
			case ShaderDataType::Mat3:     return GL_FLOAT;
			case ShaderDataType::Mat4:     return GL_FLOAT;
			case ShaderDataType::Int:      return GL_INT;
			case ShaderDataType::Int2:     return GL_INT;
			case ShaderDataType::Int3:     return GL_INT;
			case ShaderDataType::Int4:     return GL_INT;
			case ShaderDataType::Boolean:  return GL_BOOL;
		}

		SP_CORE_ASSERT(false, "Unknown Shader Data Type!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		SP_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		SP_PROFILE_FUNCTION();

		if (m_RendererID)
			glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		SP_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		SP_PROFILE_FUNCTION();

		glBindVertexArray(NULL);
	}

	void OpenGLVertexArray::AddVertexBuffer(const SharedRef<VertexBuffer>& vertexBuffer)
	{
		SP_PROFILE_FUNCTION();

		SP_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();

		for (auto& element : layout)
		{
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLDataType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset
			);
			glEnableVertexAttribArray(index);

			index++;
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const SharedRef<IndexBuffer>& indexBuffer)
	{
		SP_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}
