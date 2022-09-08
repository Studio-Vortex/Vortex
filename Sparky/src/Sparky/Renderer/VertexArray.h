#pragma once

#include "Sparky/Renderer/Buffer.h"

#include <memory>

namespace Sparky {

	class SPARKY_API VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const SharedRef<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const SharedRef<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<SharedRef<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const SharedRef<IndexBuffer>& GetIndexBuffer() const = 0;

		static VertexArray* Create();
	};

}