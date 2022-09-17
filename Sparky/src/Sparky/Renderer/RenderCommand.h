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

		inline static void DrawIndexed(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		inline static void DrawLines(const SharedRef<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}

		inline static void SetLineSize(float thickness)
		{
			s_RendererAPI->SetLineWidth(thickness);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};

}