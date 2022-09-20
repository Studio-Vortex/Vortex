#include "StatsPanel.h"

namespace Sparky {

	void StatsPanel::OnGuiRender(Entity hoveredEntity, bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Stats", &s_ShowPanel);
			const char* name = "None";
			if (hoveredEntity)
				name = hoveredEntity.GetComponent<TagComponent>().Tag.c_str();

			Gui::Text("Hovered Entity: %s", name);
			Gui::Separator();

			auto stats = Renderer2D::GetStats();
			Gui::PushFont(boldFont);
			Gui::Text("Renderer2D Stats:");
			Gui::PopFont();
			Gui::Text("Draw Calls: %i", stats.DrawCalls);
			Gui::Text("Quads:      %i", stats.QuadCount);
			Gui::Text("Triangles:  %i", stats.GetTriangleCount());
			Gui::Text("Lines:      %i", stats.LineCount);
			Gui::Text("Vertices:   %i", stats.GetVertexCount());
			Gui::Text("Indices:    %i", stats.GetIndexCount());
			Gui::Separator();

			Gui::PushFont(boldFont);
			Gui::Text("Benchmark:");
			Gui::PopFont();
			Gui::Text("Average frame time: %.3fms", 1000.0f / io.Framerate);
			Gui::Text("FPS:  %.1f", io.Framerate);
			Gui::Separator();

			const auto& rendererInfo = Renderer::GetGraphicsAPIInfo();
			Gui::PushFont(boldFont);
			Gui::Text("Graphics API Info:");
			Gui::PopFont();
			Gui::Text("API:     %s", rendererInfo.API);
			Gui::Text("GPU:     %s", rendererInfo.GPU);
			Gui::Text("Vendor:  %s", rendererInfo.Vendor);
			Gui::Text("Version: %s", rendererInfo.Version);
			Gui::Text("GLSL:    %s", rendererInfo.ShadingLanguageVersion);
			Gui::End();
		}
	}

}
