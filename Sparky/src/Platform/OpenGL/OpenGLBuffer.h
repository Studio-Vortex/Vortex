#pragma once

#include "Sparky/Core/Base.h"

#include "Sparky/Renderer/Buffer.h"

namespace Sparky {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		OpenGLVertexBuffer(uint32_t size);
		~OpenGLVertexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		void SetData(const void* data, uint32_t size) const override;

		inline const BufferLayout& GetLayout() const override { return m_Layout; }
		inline void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

	private:
		uint32_t m_RendererID = 0;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		OpenGLIndexBuffer(uint16_t* indices, uint32_t count);
		~OpenGLIndexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		inline uint32_t GetCount() const override { return m_Count; }

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_Count;
	};

}
