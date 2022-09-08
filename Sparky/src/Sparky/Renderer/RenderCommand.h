#pragma once

#include "Sparky/Core.h"

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

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const SharedRef<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};

}