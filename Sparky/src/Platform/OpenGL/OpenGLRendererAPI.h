#pragma once

#include "Sparky/Renderer/RendererAPI.h"

namespace Sparky {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() const override;

		void SetViewport(const Viewport& viewport) const override;

		void SetClearColor(const Math::vec3& color) const override;

		void SetWireframeMode(bool enabled) const override;

		void Clear() const override;

		void DrawIndexed(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount = 0) const override;
		void DrawLines(const SharedRef<VertexArray>& vertexArray, uint32_t vertexCount) const override;

		void SetLineWidth(float thickness) const override;
	};

}