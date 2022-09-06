#pragma once

#include "Sparky/Core.h"

namespace Sparky {

	enum class RendererAPI
	{
		None = 0, OpenGL = 1, Direct3D = 2, Vulkan = 3
	};

	class SPARKY_API Renderer
	{
	public:
		inline static RendererAPI GetGraphicsAPI() { return s_RendererAPI; }
		inline static void SetGraphicsAPI(const RendererAPI& api) { s_RendererAPI = api; }

	private:
		static RendererAPI s_RendererAPI;

	};

}