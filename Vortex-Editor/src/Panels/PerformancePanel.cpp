#include "PerformancePanel.h"

namespace Vortex {

	void PerformancePanel::OnGuiRender(size_t entityCount)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		Gui::Begin("Performance", &s_ShowPanel);

		uint32_t activeID = Application::Get().GetGuiLayer()->GetActiveLayerID();
		Gui::Text("Active Panel ID: %u", activeID);

		Gui::PushFont(boldFont);
		Gui::Text("Scene");
		Gui::PopFont();
		Gui::Separator();

		Gui::Text("Entity Count: %u", entityCount);

		Gui::PushFont(boldFont);
		Gui::Text("Renderer");
		Gui::PopFont();
		Gui::Separator();

		RenderTime& renderTime = Renderer::GetRenderTime();
		Gui::Text("Geometry Pass: %.4fms", renderTime.GeometryPassRenderTime);
		Gui::Text("Shadow Pass: %.4fms", renderTime.ShadowMapRenderTime);

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

		Gui::Spacing();
		Gui::PushFont(boldFont);
		Gui::Text("Frame Time");
		Gui::PopFont();
		Gui::Separator();

		Gui::Text("Average frame time: %.3fms", 1000.0f / io.Framerate);
		Gui::Text("FPS:  %.0f", io.Framerate);

		Gui::Spacing();
		Gui::PushFont(boldFont);
		Gui::Text("Graphics API");
		Gui::PopFont();
		Gui::Separator();

		const auto& rendererInfo = Renderer::GetGraphicsAPIInfo();
		Gui::Text("API:     %s", rendererInfo.API);
		Gui::Text("GPU:     %s", rendererInfo.GPU);
		Gui::Text("Vendor:  %s", rendererInfo.Vendor);
		Gui::Text("Version: %s", rendererInfo.Version);
		Gui::Text("GLSL:    %s", rendererInfo.ShadingLanguageVersion);

		Gui::End();
	}

}
