#include "vxpch.h"
#include "Application.h"

#include "Vortex/Core/Input/Input.h"
#include "Vortex/Core/Thread/ThreadPool.h"

#include "Vortex/Events/KeyEvent.h"

#include "Vortex/Audio/Audio.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Renderer/ParticleSystem/ParticleSystem.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/2D/Physics2D.h"

#include "Vortex/Network/Networking.h"

#include "Vortex/System/SystemManager.h"

#include "Vortex/Utils/FileSystem.h"
#include "Vortex/Utils/Random.h"
#include "Vortex/Utils/Time.h"

extern bool g_ApplicationRunning;

namespace Vortex {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationProperties& props)
		: m_Properties(props)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		SetWorkingDirectory();
		CreateWindowV();
		InitializeSubModules();

		m_Running = true;
	}

	Application::~Application()
	{
		VX_PROFILE_FUNCTION();

		// Note:
		// We have to explicitly destroy the layer stack before
		// the engine's submodules are shutdown. This is mainly because
		// the Audio system needs to outlive the lifetime of the layer stack.
		// It is also just a good idea to destroy all the layers before submodules
		// get shutdown because something in the layers code may rely on a submodule
		m_LayerStack.~LayerStack();

		ShutdownSubModules();
	}

	void Application::SetWorkingDirectory()
	{
		VX_PROFILE_FUNCTION();

		if (!m_Properties.WorkingDirectory.empty())
		{
			FileSystem::SetWorkingDirectory(m_Properties.WorkingDirectory);
		}
		else
		{
			m_Properties.WorkingDirectory = FileSystem::GetWorkingDirectory().string();
		}
	}

	void Application::CreateWindowV()
	{
		VX_PROFILE_FUNCTION();

		WindowProperties windowProps;
		windowProps.Size.x = m_Properties.WindowWidth;
		windowProps.Size.y = m_Properties.WindowHeight;
		windowProps.Title = m_Properties.Name;
		windowProps.Maximized = m_Properties.MaximizeWindow;
		windowProps.Decorated = m_Properties.WindowDecorated;
		windowProps.Resizeable = m_Properties.WindowResizable;
		windowProps.VSync = m_Properties.VSync;

		// We need to set the app's graphics api before we create the window
		Renderer::SetGraphicsAPI(m_Properties.GraphicsAPI);

		m_Window = Window::Create(windowProps);

		if (!m_Properties.MaximizeWindow)
		{
			m_Window->CenterWindow();
		}

		m_Window->SetEventCallback(VX_BIND_CALLBACK(Application::OnEvent));
	}

	void Application::InitializeSubModules()
	{
		VX_PROFILE_FUNCTION();

		// ThreadPool::Init();
		Networking::Init();
		Renderer::Init();
		SystemManager::RegisterAssetSystem<ParticleSystem>();
		Physics::Init();
		Audio::Init();
		Random::Init();
		Font::Init();
		Input::Init();

		if (m_Properties.EnableGUI)
		{
			m_GuiLayer = new GuiLayer();
			PushOverlay(m_GuiLayer);
		}
	}

	void Application::ShutdownSubModules()
	{
		VX_PROFILE_FUNCTION();

		Input::Shutdown();
		Audio::Shutdown();
		Physics::Shutdown();
		Font::Shutdown();
		SystemManager::UnregisterAssetSystem<ParticleSystem>();
		Renderer::Shutdown();
		Networking::Shutdown();
		// ThreadPool::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		VX_PROFILE_FUNCTION();

		layer->OnAttach();
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		VX_PROFILE_FUNCTION();

		overlay->OnAttach();
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::Close()
	{
		m_Running = false;
		g_ApplicationRunning = false;
	}

	void Application::OnEvent(Event& e)
	{
		VX_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(VX_BIND_CALLBACK(Application::OnWindowCloseEvent));
		dispatcher.Dispatch<WindowResizeEvent>(VX_BIND_CALLBACK(Application::OnWindowResizeEvent));

		Input::OnEvent(e);

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;

			(*it)->OnEvent(e);
		}
	}

	void Application::SubmitToMainThreadQueue(const std::function<void()>& func)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		m_MainThreadQueue.emplace_back(func);
	}

	void Application::AddModule(const SubModule& submodule)
	{
		m_ModuleLibrary.Add(submodule);
	}

	void Application::RemoveModule(const SubModule& submodule)
	{
		const std::string& moduleName = submodule.GetName();
		m_ModuleLibrary.Remove(moduleName);
	}

	const ModuleLibrary& Application::GetModules() const
	{
		return m_ModuleLibrary;
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (const auto& func : m_MainThreadQueue)
		{
			func();
		}

		m_MainThreadQueue.clear();
	}

	void Application::Run()
	{
		VX_PROFILE_FUNCTION();

		while (m_Running)
		{
			VX_PROFILE_SCOPE("Application Loop");

			float time = Time::GetTime();
			TimeStep delta = time - m_LastFrameTime;
			Time::SetDeltaTime(delta);
			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			if (!m_ApplicationMinimized)
			{
				VX_PROFILE_SCOPE("LayerStack OnUpdate");

				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(delta);
				}
			}

			if (m_Properties.EnableGUI)
			{
				m_GuiLayer->BeginFrame();

				{
					VX_PROFILE_SCOPE("LayerStack OnGuiRender");

					for (Layer* layer : m_LayerStack)
					{
						layer->OnGuiRender();
					}
				}

				m_GuiLayer->EndFrame();
			}

			Input::ResetChangesForNextFrame();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowCloseEvent(WindowCloseEvent& e)
	{
		Close();

		return false;
	}

	bool Application::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		VX_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_ApplicationMinimized = true;
			return false;
		}

		m_ApplicationMinimized = false;

		Viewport viewport;
		viewport.TopLeftXPos = 0;
		viewport.TopLeftYPos = 0;
		viewport.Width = e.GetWidth();
		viewport.Height = e.GetHeight();

		Renderer::OnWindowResize(viewport);

		return false;
	}

}
