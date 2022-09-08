#pragma once

#include "Sparky/Renderer/RendererAPI.h"

namespace Sparky {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() const override;

		void SetViewport(const Viewport& viewport) const override;

		void SetClearColor(const Math::vec3& color) override;
		void Clear() const override;

		void DrawIndexed(const SharedRef<VertexArray>& vertexArray) const override;
	};

}