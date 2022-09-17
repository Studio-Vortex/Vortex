#pragma once

#include "Sparky/Core/Base.h"

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

	class SPARKY_API RendererAPI
	{
	public:
		enum class SPARKY_API API
		{
			None = 0, OpenGL = 1, Direct3D = 2, Vulkan = 3
		};

		struct RendererInfo
		{
			const char* API;
			const unsigned char* GPU;
			const unsigned char* Vendor;
			const unsigned char* Version;
			const unsigned char* ShadingLanguageVersion;
		};

	public:
		virtual void Init() const = 0;

		virtual void SetViewport(const Viewport& viewport) const = 0;

		virtual void SetClearColor(const Math::vec3& color) const = 0;

		virtual void SetWireframeMode(bool enabled) const = 0;

		virtual void Clear() const = 0;

		virtual void DrawIndexed(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount = 0) const = 0;
		virtual void DrawLines(const SharedRef<VertexArray>& vertexArray, uint32_t vertexCount) const = 0;

		virtual void SetLineWidth(float thickness) const = 0;

		inline static API GetAPI() { return s_API; }
		inline static void SetAPI(API api) { s_API = api; }

		inline static const RendererInfo& GetAPIInfo() { return s_RendererInfo; }
		inline static void SetAPIInfo(const RendererInfo& info) { s_RendererInfo = info; }

	private:
		static API s_API;
		static RendererInfo s_RendererInfo;
	};

}