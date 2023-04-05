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

		void AddVertexBuffer(SharedReference<VertexBuffer>& vertexBuffer) override;
		void SetIndexBuffer(SharedReference<IndexBuffer>& indexBuffer) override;

		inline const std::vector<SharedReference<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		inline const SharedReference<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<SharedReference<VertexBuffer>> m_VertexBuffers;
		SharedReference<IndexBuffer> m_IndexBuffer;
	};

}
