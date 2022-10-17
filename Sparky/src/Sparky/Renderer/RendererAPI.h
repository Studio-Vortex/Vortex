#pragma once

#include "Sparky/Core/Base.h"

#include "Sparky/Core/Math.h"
#include "Sparky/Renderer/VertexArray.h"

namespace Sparky {

#ifdef SP_DEBUG
	#define SP_RENDERER_STATISTICS 1;
#elif SP_RELEASE
	#define SP_RENDERER_STATISTICS 1; // Temporary
#endif // SP_DEBUG

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

	struct SPARKY_API Viewport
	{
		uint32_t TopLeftXPos = 0;
		uint32_t TopLeftYPos = 0;
		uint32_t Width = 0;
		uint32_t Height = 0;
	};

	class SPARKY_API RendererAPI
	{
	public:
		enum class SPARKY_API API
		{
			None = 0, OpenGL = 1, Direct3D = 2, Vulkan = 3
		};

		enum class TriangleCullMode
		{
			None = 0, Front, Back, FrontAndBack
		};

		struct SPARKY_API RendererInfo
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

		virtual void SetCullMode(TriangleCullMode cullMode) const = 0;

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
				case Sparky::RendererAPI::TriangleCullMode::None:         return "None";
				case Sparky::RendererAPI::TriangleCullMode::Front:        return "Front";
				case Sparky::RendererAPI::TriangleCullMode::Back:         return "Back";
				case Sparky::RendererAPI::TriangleCullMode::FrontAndBack: return "Front And Back";
			}

			SP_CORE_ASSERT(false, "Unknown Triangle Cull Mode!");
			return "None";
		}

	}

}
