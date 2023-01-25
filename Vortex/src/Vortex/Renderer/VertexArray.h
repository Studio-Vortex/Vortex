#pragma once

#include "Vortex/Renderer/Buffer.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

namespace Vortex {

	class VORTEX_API VertexArray : public RefCounted
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const SharedRef<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const SharedRef<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<SharedRef<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const SharedRef<IndexBuffer>& GetIndexBuffer() const = 0;

		static SharedRef<VertexArray> Create();
	};

}