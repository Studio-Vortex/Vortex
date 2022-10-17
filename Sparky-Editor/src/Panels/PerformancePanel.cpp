#include "PerformancePanel.h"

namespace Sparky {

	void PerformancePanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Performance", &s_ShowPanel);

			if (Gui::TreeNodeEx("Renderer Statistics"))
			{
				if (Gui::BeginTable("Renderer", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					Gui::TableSetupColumn("3D Renderer");
					Gui::TableSetupColumn("2D Renderer");
					Gui::TableHeadersRow();
				
					// Renderer Stats
					{
						Gui::TableNextColumn();
						auto stats = Renderer::GetStats();
						Gui::Text("Draw Calls: %i", stats.DrawCalls);
						Gui::Text("Quads:      %i", stats.QuadCount);
						Gui::Text("Triangles:  %i", stats.GetTriangleCount());
						Gui::Text("Lines:      %i", stats.LineCount);
						Gui::Text("Vertices:   %i", stats.GetVertexCount());
						Gui::Text("Indices:    %i", stats.GetIndexCount());
					}

					// Renderer 2D Stats
					{
						Gui::TableNextColumn();
						auto stats = Renderer2D::GetStats();
						Gui::Text("Draw Calls: %i", stats.DrawCalls);
						Gui::Text("Quads:      %i", stats.QuadCount);
						Gui::Text("Triangles:  %i", stats.GetTriangleCount());
						Gui::Text("Lines:      %i", stats.LineCount);
						Gui::Text("Vertices:   %i", stats.GetVertexCount());
						Gui::Text("Indices:    %i", stats.GetIndexCount());
					}

					Gui::EndTable();
				}

				Gui::TreePop();
			}

			if (Gui::TreeNodeEx("Frame Time"))
			{
				Gui::Text("Average frame time: %.3fms", 1000.0f / io.Framerate);
				Gui::Text("FPS:  %.0f", io.Framerate);

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
