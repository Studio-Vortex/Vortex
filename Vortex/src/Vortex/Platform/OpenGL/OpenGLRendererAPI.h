#pragma once

#include "Vortex/Renderer/RendererAPI.h"

namespace Vortex {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() const override;

		void SetViewport(const Viewport& viewport) const override;

		void SetClearColor(const Math::vec3& color) const override;

		void SetWireframeMode(bool enabled) const override;

		void Clear() const override;

		void DrawTriangles(SharedReference<VertexArray>& vertexArray, uint32_t vertexCount) const override;
		void DrawIndexed(SharedReference<VertexArray>& vertexArray, uint32_t indexCount = 0) const override;
		void DrawLines(SharedReference<VertexArray>& vertexArray, uint32_t vertexCount) const override;
		void DrawTriangleStrip(SharedReference<VertexArray>& vertexArray, uint32_t indexCount) const override;

		void EnableDepthTest() const override;
		void DisableDepthTest() const override;

		void SetBlendMode(RendererAPI::BlendMode blendMode) const override;

		void EnableStencilTest() const override;
		void DisableStencilTest() const override;

		void EnableDepthMask() const override;
		void DisableDepthMask() const override;

		void SetLineWidth(float thickness) const override;

		void SetCullMode(TriangleCullMode cullMode) const override;

		void SetStencilOperation(StencilOperation failOperation, StencilOperation zFailOperation, StencilOperation passOperation) const override;
		void SetStencilFunc(StencilOperation func, int ref, int mask) const override;
		void SetStencilMask(int mask) const override;
	};

}
