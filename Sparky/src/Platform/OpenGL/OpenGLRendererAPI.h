#pragma once

#include "Sparky/Renderer/RendererAPI.h"

namespace Sparky {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void SetClearColor(const Math::vec3& color);
		void Clear() const;

		void DrawIndexed(const SharedRef<VertexArray>& vertexArray) const;
	};

}