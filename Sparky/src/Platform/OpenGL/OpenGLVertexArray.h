#pragma once

#include "Sparky/Renderer/VertexArray.h"

namespace Sparky {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray() override;

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(const SharedRef<VertexBuffer>& vertexBuffer) override;
		void SetIndexBuffer(const SharedRef<IndexBuffer>& indexBuffer) override;

		inline const std::vector<SharedRef<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		inline const SharedRef<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

	private:
		uint32_t m_RendererID;
		std::vector<SharedRef<VertexBuffer>> m_VertexBuffers;
		SharedRef<IndexBuffer> m_IndexBuffer;
	};

}
