#include "PerformancePanel.h"

namespace Sparky {

	void PerformancePanel::OnGuiRender(Entity hoveredEntity, bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Performance", &s_ShowPanel);

			auto stats = Renderer2D::GetStats();
			if (Gui::TreeNodeEx("Renderer 2D Statistics"))
			{
				Gui::Text("Draw Calls: %i", stats.DrawCalls);
				Gui::Text("Quads:      %i", stats.QuadCount);
				Gui::Text("Triangles:  %i", stats.GetTriangleCount());
				Gui::Text("Lines:      %i", stats.LineCount);
				Gui::Text("Vertices:   %i", stats.GetVertexCount());
				Gui::Text("Indices:    %i", stats.GetIndexCount());

				Gui::TreePop();
			}

			if (Gui::TreeNodeEx("Framerate"))
			{
				Gui::Text("Average frame time: %.3fms", 1000.0f / io.Framerate);
				Gui::Text("FPS:  %.1f", io.Framerate);

				Gui::TreePop();
			}

			const auto& rendererInfo = Renderer::GetGraphicsAPIInfo();
			if (Gui::TreeNodeEx("Graphics API"))
			{
				Gui::Text("API:     %s", rendererInfo.API);
				Gui::Text("GPU:     %s", rendererInfo.GPU);
				Gui::Text("Vendor:  %s", rendererInfo.Vendor);
				Gui::Text("Version: %s", rendererInfo.Version);
				Gui::Text("GLSL:    %s", rendererInfo.ShadingLanguageVersion);

				Gui::TreePop();
			}

			Gui::End();
		}
	}

}
