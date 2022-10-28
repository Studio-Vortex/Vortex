#pragma once

#include "Sparky/Core/Base.h"

#include "Sparky/Renderer/RendererAPI.h"

namespace Sparky {

	class SPARKY_API RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetViewport(const Viewport& viewport)
		{
			s_RendererAPI->SetViewport(viewport);
		}

		inline static void SetClearColor(const Math::vec3& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		inline static void SetWireframe(bool enabled)
		{
			s_RendererAPI->SetWireframeMode(enabled);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawTriangles(const SharedRef<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_RendererAPI->DrawTriangles(vertexArray, vertexCount);
		}

		inline static void DrawIndexed(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		inline static void DrawLines(const SharedRef<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}

		inline static void DrawTriangleStrip(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount)
		{
			s_RendererAPI->DrawTriangleStrip(vertexArray, indexCount);
		}

		inline static void EnableDepthMask()
		{
			s_RendererAPI->EnableDepthMask();
		}

		inline static void DisableDepthMask()
		{
			s_RendererAPI->DisableDepthMask();
		}

		inline static void SetLineSize(float thickness)
		{
			s_RendererAPI->SetLineWidth(thickness);
		}

		inline static void SetCullMode(RendererAPI::TriangleCullMode cullMode)
		{
			s_RendererAPI->SetCullMode(cullMode);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};

}
