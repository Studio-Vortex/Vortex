#pragma once

#include "Vortex/Renderer/Buffer.h"

#include "Vortex/ReferenceCounting/RefCounted.h"
#include "Vortex/ReferenceCounting/SharedRef.h"

namespace Vortex {

	class VORTEX_API VertexArray : public RefCounted
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(SharedReference<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(SharedReference<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<SharedReference<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const SharedReference<IndexBuffer>& GetIndexBuffer() const = 0;

		static SharedReference<VertexArray> Create();
	};

}