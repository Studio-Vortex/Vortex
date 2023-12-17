#include "PerformancePanel.h"

#include <Vortex/Scripting/ScriptEngine.h>

namespace Vortex {

	void PerformancePanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		Gui::Begin("Performance", &IsOpen);

		Application& application = Application::Get();

		const GuiLayer* guiLayer = application.GetGuiLayer();
		const uint32_t activeID = guiLayer->GetActiveLayerID();
		Gui::Text("Active Panel ID: %u", activeID);

		DrawHeading("Scene");
		const size_t entityCount = m_ContextScene->GetEntityCount();
		Gui::Text("Entity Count: %u", entityCount);
		const size_t scriptInstanceCount = ScriptEngine::GetScriptInstanceCount();
		Gui::Text("Active Script Entities: %u", scriptInstanceCount);

		DrawHeading("Renderer Frame Time");
		const ImGuiIO& io = Gui::GetIO();
		Gui::Text("Average Frame Time: %.3fms", 1000.0f / io.Framerate);
		Gui::Text("FPS:  %.0f", io.Framerate);

		const RenderTime& renderTime = Renderer::GetRenderTime();
		Gui::Text("Pre Geometry Pass Sort: %.4fms", renderTime.PreGeometryPassSortTime);
		Gui::Text("Geometry Pass: %.4fms", renderTime.GeometryPassRenderTime);
		Gui::Text("Shadow Pass: %.4fms", renderTime.ShadowMapRenderTime);
		Gui::Text("Bloom Pass: %.4fms", renderTime.BloomPassRenderTime);

		DrawHeading("Application Frame Time");
		const FrameTime& frameTime = application.GetFrameTime();
		Gui::Text("Script Update: %.4fms", frameTime.ScriptUpdateTime);
		Gui::Text("Physics Update: %.4fms", frameTime.PhysicsUpdateTime);

		DrawHeading("Input Assembly");
		RenderStatistics stats = Renderer::GetStats();
		const RenderStatistics temp = Renderer2D::GetStats();

		stats.DrawCalls += temp.DrawCalls;
		stats.QuadCount += temp.QuadCount;
		stats.LineCount += temp.LineCount;

		Gui::Text("Draw Calls: %i", stats.DrawCalls);
		Gui::Text("Quads:      %i", stats.QuadCount);
		Gui::Text("Triangles:  %i", stats.GetTriangleCount());
		Gui::Text("Vertices:   %i", stats.GetVertexCount());
		Gui::Text("Indices:    %i", stats.GetIndexCount());

		DrawHeading("Graphics API");
		const RendererAPI::Info& rendererInfo = Renderer::GetGraphicsAPIInfo();
		Gui::Text("API:     %s", rendererInfo.Name);
		Gui::Text("GPU:     %s", rendererInfo.GPU);
		Gui::Text("Vendor:  %s", rendererInfo.Vendor);
		Gui::Text("Version: %s", rendererInfo.Version);
		Gui::Text("GLSL:    %s", rendererInfo.ShadingLanguageVersion);

		Gui::End();
	}

	void PerformancePanel::DrawHeading(const char* title)
	{
		Gui::Spacing();
		UI::PushFont("Bold");
		Gui::Text(title);
		UI::PopFont();
		UI::Draw::Underline();
	}

}
