#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/Math.h"
#include "Vortex/Renderer/VertexArray.h"

namespace Vortex {

#ifdef VX_DEBUG
	#define SP_RENDERER_STATISTICS 1;
#elif VX_RELEASE
	#define SP_RENDERER_STATISTICS 1; // Temporary
#endif // VX_DEBUG

	static constexpr uint32_t VERTICES_PER_CUBE = 24;
	static constexpr uint32_t INDICES_PER_CUBE = 36;

	static constexpr uint32_t VERTICES_PER_QUAD = 4;
	static constexpr uint32_t INDICES_PER_QUAD = 6;

	struct RenderStatistics
	{
		uint32_t DrawCalls;
		uint32_t QuadCount;
		uint32_t LineCount;

		uint32_t GetTriangleCount() const { return QuadCount * 2; }
		uint32_t GetVertexCount() const { return (QuadCount * VERTICES_PER_QUAD) + (LineCount * 2); }
		uint32_t GetIndexCount() const { return QuadCount * INDICES_PER_QUAD + (LineCount * 2); }
	};

	struct VORTEX_API Viewport
	{
		uint32_t TopLeftXPos = 0;
		uint32_t TopLeftYPos = 0;
		uint32_t Width = 0;
		uint32_t Height = 0;
	};

	class VORTEX_API RendererAPI
	{
	public:
		enum class VORTEX_API API
		{
			None = 0, OpenGL = 1, Direct3D = 2, Vulkan = 3
		};

		enum class TriangleCullMode
		{
			None = 0, Front, Back, FrontAndBack
		};

		enum class StencilOperation
		{
			None = 0, Keep, Always, NotEqual, Replace
		};

		struct VORTEX_API RendererInfo
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

		virtual void DrawTriangles(const SharedRef<VertexArray>& vertexArray, uint32_t vertexCount) const = 0;
		virtual void DrawIndexed(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount = 0) const = 0;
		virtual void DrawLines(const SharedRef<VertexArray>& vertexArray, uint32_t vertexCount) const = 0;
		virtual void DrawTriangleStrip(const SharedRef<VertexArray>& vertexArray, uint32_t indexCount) const = 0;

		virtual void EnableDepthTest() const = 0;

		virtual void EnableDepthMask() const = 0;
		virtual void DisableDepthMask() const = 0;

		virtual void SetLineWidth(float thickness) const = 0;

		virtual void SetCullMode(TriangleCullMode cullMode) const = 0;

		virtual void SetStencilOperation(StencilOperation failOperation, StencilOperation zFailOperation, StencilOperation passOperation) const = 0;
		virtual void SetStencilFunc(StencilOperation func, int ref, int mask) const = 0;
		virtual void SetStencilMask(int mask) const = 0;

		inline static API GetAPI() { return s_API; }
		inline static void SetAPI(API api) { s_API = api; }

		inline static const RendererInfo& GetAPIInfo() { return s_RendererInfo; }
		inline static void SetAPIInfo(const RendererInfo& info) { s_RendererInfo = info; }

	private:
		static API s_API;
		static RendererInfo s_RendererInfo;
	};

	namespace Utils {

		static const char* TriangleCullModeToString(RendererAPI::TriangleCullMode cullMode)
		{
			switch (cullMode)
			{
				case Vortex::RendererAPI::TriangleCullMode::None:         return "None";
				case Vortex::RendererAPI::TriangleCullMode::Front:        return "Front";
				case Vortex::RendererAPI::TriangleCullMode::Back:         return "Back";
				case Vortex::RendererAPI::TriangleCullMode::FrontAndBack: return "Front And Back";
			}

			VX_CORE_ASSERT(false, "Unknown Triangle Cull Mode!");
			return "None";
		}

	}

}
