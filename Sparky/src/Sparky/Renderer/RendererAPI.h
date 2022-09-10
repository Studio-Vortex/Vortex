#pragma once

#include "Sparky/Core/Math.h"
#include "Sparky/Renderer/VertexArray.h"

namespace Sparky {

	struct Viewport
	{
		uint32_t XPos;
		uint32_t YPos;
		uint32_t Width;
		uint32_t Height;
	};

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1, Direct3D = 2, Vulkan = 3
		};

	public:
		virtual void Init() const = 0;

		virtual void SetViewport(const Viewport& viewport) const = 0;

		virtual void SetClearColor(const Math::vec3& color) const = 0;
		virtual void Clear() const = 0;

		virtual void DrawIndexed(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount = 0) const = 0;

		inline static API GetAPI() { return s_API; }
		inline static void SetAPI(API api) { s_API = api; }

	private:
		static API s_API;
	};

}