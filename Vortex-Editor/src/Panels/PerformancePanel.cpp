#include "PerformancePanel.h"

namespace Vortex {

	void PerformancePanel::OnGuiRender(size_t entityCount)
	{
		ImGuiIO& io = Gui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		Gui::Begin("Performance", &s_ShowPanel);

		uint32_t activeID = Application::Get().GetGuiLayer()->GetActiveLayerID();
		Gui::Text("Active Panel ID: %u", activeID);

		DrawHeading("Scene");

		Gui::Text("Entity Count: %u", entityCount);

		DrawHeading("Renderer Frame Time");

		Gui::Text("Average Frame Time: %.3fms", 1000.0f / io.Framerate);
		Gui::Text("FPS:  %.0f", io.Framerate);

		RenderTime& renderTime = Renderer::GetRenderTime();
		Gui::Text("Pre Geometry Pass Sort: %.4fms", renderTime.PreGeometryPassSortTime);
		Gui::Text("Geometry Pass: %.4fms", renderTime.GeometryPassRenderTime);
		Gui::Text("Shadow Pass: %.4fms", renderTime.ShadowMapRenderTime);
		Gui::Text("Bloom Pass: %.4fms", renderTime.BloomPassRenderTime);

		static const char* columns[] = {"Renderer", "Renderer2D"};

		UI::Table("Input Assembly Info", columns, VX_ARRAYCOUNT(columns), Gui::GetContentRegionAvail(), []()
		{
			Gui::TableNextColumn();
			auto stats = Renderer::GetStats();
			Gui::Text("Draw Calls: %i", stats.DrawCalls);
			Gui::Text("Quads:      %i", stats.QuadCount);
			Gui::Text("Triangles:  %i", stats.GetTriangleCount());
			Gui::Text("Lines:      %i", stats.LineCount);
			Gui::Text("Vertices:   %i", stats.GetVertexCount());
			Gui::Text("Indices:    %i", stats.GetIndexCount());

			Gui::TableNextColumn();
			stats = Renderer2D::GetStats();
			Gui::Text("Draw Calls: %i", stats.DrawCalls);
			Gui::Text("Quads:      %i", stats.QuadCount);
			Gui::Text("Triangles:  %i", stats.GetTriangleCount());
			Gui::Text("Lines:      %i", stats.LineCount);
			Gui::Text("Vertices:   %i", stats.GetVertexCount());
			Gui::Text("Indices:    %i", stats.GetIndexCount());
		});

		DrawHeading("Graphics API");

		const auto& rendererInfo = Renderer::GetGraphicsAPIInfo();
		Gui::Text("API:     %s", rendererInfo.Name);
		Gui::Text("GPU:     %s", rendererInfo.GPU);
		Gui::Text("Vendor:  %s", rendererInfo.Vendor);
		Gui::Text("Version: %s", rendererInfo.Version);
		Gui::Text("GLSL:    %s", rendererInfo.ShadingLanguageVersion);

		Gui::End();
	}

	void PerformancePanel::DrawHeading(const char* title)
	{
		auto boldFont = Gui::GetIO().Fonts->Fonts[0];

		Gui::Spacing();
		Gui::PushFont(boldFont);
		Gui::Text(title);
		Gui::PopFont();
		UI::Draw::Underline();
	}

}
