#pragma once

#include "Vortex/Renderer/VertexArray.h"

namespace Vortex {

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
		uint32_t m_RendererID = 0;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<SharedRef<VertexBuffer>> m_VertexBuffers;
		SharedRef<IndexBuffer> m_IndexBuffer;
	};

}
