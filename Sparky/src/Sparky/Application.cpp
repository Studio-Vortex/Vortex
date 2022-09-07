#include "sppch.h"
#include "Application.h"

#include "Sparky/Events/Event.h"
#include "Sparky/Events/MouseEvent.h"

#include "Sparky/Input.h"
#include "Sparky/Renderer/Renderer.h"
#include <Glad/glad.h>

namespace Sparky {

	Application* Application::s_Instance = nullptr;

	Application::Application()
		: m_Window(Window::Create()), m_GuiLayer(new GuiLayer())
	{
		SP_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window->SetEventCallback(SP_BIND_CALLBACK(Application::OnEvent));

		PushOverlay(m_GuiLayer);

		m_TriangleVA.reset(VertexArray::Create());

		float vertices[3 * 7] = {
		   //position           color
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		};
		std::shared_ptr<VertexBuffer> pTriangleVB;
		pTriangleVB.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" },
		};

		pTriangleVB->SetLayout(layout);
		m_TriangleVA->AddVertexBuffer(pTriangleVB);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<IndexBuffer> pTriangleIB;
		pTriangleIB.reset(IndexBuffer::Create(indices, SP_ARRAYCOUNT(indices)));
		m_TriangleVA->SetIndexBuffer(pTriangleIB);

		m_SquareVA.reset(VertexArray::Create());

		float squareVertices[4 * 7] = {
			//position
			 -0.75f, -0.75f, 0.0f, // bottom left
			  0.75f, -0.75f, 0.0f, // bottom right
			  0.75f,  0.75f, 0.0f, // top right
			 -0.75f,  0.75f, 0.0f, // top left
		};

		std::shared_ptr<VertexBuffer> pSquareVB;
		pSquareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		pSquareVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
		});
		m_SquareVA->AddVertexBuffer(pSquareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
		std::shared_ptr<IndexBuffer> pSquareIB;
		pSquareIB.reset(IndexBuffer::Create(squareIndices, SP_ARRAYCOUNT(squareIndices)));
		m_SquareVA->SetIndexBuffer(pSquareIB);

		std::string vertexSrc = R"(
			#version 460 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec4 f_Color;

			void main()
			{
				gl_Position = vec4(a_Position, 1.0);
				f_Color = a_Color;
			}
		)";

		std::string fragmentSrc = R"(
			#version 460 core

			out vec4 gl_Color;
			
			in vec4 f_Color;

			void main()
			{
				gl_Color = f_Color;
			}
		)";

		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));

		std::string vertexSrc2 = R"(
			#version 460 core

			layout(location = 0) in vec3 a_Position;

			out vec3 f_Position;

			void main()
			{
				f_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc2 = R"(
			#version 460 core

			out vec4 gl_Color;
			
			in vec3 f_Position;

			void main()
			{
				gl_Color = vec4(f_Position * 0.5 + 0.5, 1.0);
			}
		)";

		m_Shader2.reset(new Shader(vertexSrc2, fragmentSrc2));
	}

	Application::~Application()
	{

	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(SP_BIND_CALLBACK(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);

			if (e.Handled)
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{
			glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Shader2->Enable();
			m_SquareVA->Bind();
			glDrawElements(GL_TRIANGLES, m_SquareVA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			m_Shader->Enable();
			m_TriangleVA->Bind();
			glDrawElements(GL_TRIANGLES, m_TriangleVA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			m_GuiLayer->BeginFrame();

			for (Layer* layer : m_LayerStack)
				layer->OnGuiRender();

			m_GuiLayer->EndFrame();

			m_Window->OnUpdate();
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}