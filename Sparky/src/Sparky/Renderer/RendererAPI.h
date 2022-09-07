#pragma once

#include "Sparky/Math.h"
#include "Sparky/Renderer/VertexArray.h"

namespace Sparky {

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1, Direct3D = 2, Vulkan = 3
		};

	public:
		virtual void SetClearColor(const Math::vec3& color) = 0;
		virtual void Clear() const = 0;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) const = 0;

		inline static API GetAPI() { return s_API; }
		inline static void SetAPI(API api) { s_API = api; }

	private:
		static API s_API;
	};

}