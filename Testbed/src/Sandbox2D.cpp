#include "Sandbox2D.h"

#include <chrono>

template<typename Fn>
class Timer
{
public:
	Timer(const char* name, Fn&& func)
		: m_Name(name), m_Func(func), m_Stopped(false)
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		if (!m_Stopped)
			Stop();
	}

	void Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		m_Stopped = true;

		float duration = (end - start) * 0.001f;
		m_Func({ m_Name, duration });
	}
private:
	const char* m_Name;
	Fn m_Func;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
	bool m_Stopped;
};

#define SP_PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); })

Sandbox2D::Sandbox2D()
	:
	Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true),
	m_SquareColor(Sparky::ColorToVec4(Sparky::Color::LightGreen)),
	m_GridColor(Sparky::ColorToVec4(Sparky::Color::LightBlue)), m_GridScale(5)
{ }

void Sandbox2D::OnAttach()
{
	m_GridTexture = Sparky::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
	Sparky::Renderer2D::Shutdown();
}

void Sandbox2D::OnUpdate(Sparky::TimeStep ts)
{
	SP_PROFILE_SCOPE("Sandbox2D::OnUpdate");

	// Update
	{
		SP_PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdate(ts);
	}

	if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
		Sparky::Application::Get().CloseApplication();

	// Render
	{
		SP_PROFILE_SCOPE("Renderer Prep");
		Sparky::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
		Sparky::RenderCommand::Clear();
	}

	{
		SP_PROFILE_SCOPE("Renderer Draw Calls");
		Sparky::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Sparky::Renderer2D::DrawQuad(Math::vec3(0.0f, 0.0f, 0.1f), Math::vec2(1.0f), m_SquareColor);
		Sparky::Renderer2D::DrawQuad(Math::vec3(1.5f, 0.5f, 0.1f), Math::vec2(1.0f, 2.0f), Sparky::Color::LightRed);
		Sparky::Renderer2D::DrawQuad(Math::vec3(0.0f), Math::vec2(10.0f), m_GridTexture, 0.0f, m_GridScale, m_GridColor);
		Sparky::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnGuiRender()
{
	static bool show = true;
	Gui::Begin("Settings", &show);
	Gui::ColorEdit4("Grid Color", Math::ValuePtr(m_GridColor));
	Gui::SliderInt("Grid Scale", &m_GridScale, 1, 20);
	Gui::ColorEdit4("Square Color", Math::ValuePtr(m_SquareColor));
	Gui::Spacing();
	
	for (auto& result : m_ProfileResults)
	{
		char label[50];
		strcpy(label, "%.3fms ");
		strcat(label, result.Name);

		Gui::Text(label, result.Time);
	}

	m_ProfileResults.clear();

	Gui::End();
}

void Sandbox2D::OnEvent(Sparky::Event& e)
{
	m_CameraController.OnEvent(e);
}
