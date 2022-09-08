#include <Sparky.h>

#include "Platform/OpenGL/OpenGLShader.h"

class ExampleLayer : public Sparky::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
	{
		m_TriangleVA.reset(Sparky::VertexArray::Create());

		float vertices[3 * 7] = {
			//position           color
			 -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			  0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		};

		Sparky::SharedRef<Sparky::VertexBuffer> pTriangleVB;
		pTriangleVB = Sparky::VertexBuffer::Create(vertices, sizeof(vertices));

		Sparky::BufferLayout layout = {
			{ Sparky::ShaderDataType::Float3, "a_Position" },
			{ Sparky::ShaderDataType::Float4, "a_Color" },
		};

		pTriangleVB->SetLayout(layout);
		m_TriangleVA->AddVertexBuffer(pTriangleVB);

		uint32_t indices[3] = { 0, 1, 2 };
		Sparky::SharedRef<Sparky::IndexBuffer> pTriangleIB;
		pTriangleIB = Sparky::IndexBuffer::Create(indices, SP_ARRAYCOUNT(indices));
		m_TriangleVA->SetIndexBuffer(pTriangleIB);

		m_SquareVA.reset(Sparky::VertexArray::Create());

		float squareVertices[4 * 5] = {
			//position           tex coords
			 -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,// bottom left
			  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,// bottom right
			  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,// top right
			 -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,// top left
		};

		Sparky::SharedRef<Sparky::VertexBuffer> pSquareVB;
		pSquareVB = Sparky::VertexBuffer::Create(squareVertices, sizeof(squareVertices));

		pSquareVB->SetLayout({
			{ Sparky::ShaderDataType::Float3, "a_Position" },
			{ Sparky::ShaderDataType::Float2, "a_TexCoord" },
		});
		m_SquareVA->AddVertexBuffer(pSquareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
		Sparky::SharedRef<Sparky::IndexBuffer> pSquareIB;
		pSquareIB = Sparky::IndexBuffer::Create(squareIndices, SP_ARRAYCOUNT(squareIndices));
		m_SquareVA->SetIndexBuffer(pSquareIB);

		std::string vertPosColorVertexSrc = R"(
			#version 460 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 f_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			void main()
			{
				f_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string vertPosColorFragmentSrc = R"(
			#version 460 core

			layout (location = 0) out vec4 gl_Color;
			
			in vec3 f_Position;

			void main()
			{
				gl_Color = vec4(f_Position * 0.5 + 0.5, 1.0);
			}
		)";

		m_TriangleShader = Sparky::Shader::Create("VertexPosColor", vertPosColorVertexSrc, vertPosColorFragmentSrc);

		std::string flatColorVertexSrc = R"(
			#version 460 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			void main()
			{
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColorFragmentSrc = R"(
			#version 460 core

			layout (location = 0) out vec4 gl_Color;

			uniform vec3 u_Color;

			void main()
			{
				gl_Color = vec4(u_Color, 1.0);
			}
		)";

		m_FlatColorShader = Sparky::Shader::Create("Flat Color", flatColorVertexSrc, flatColorFragmentSrc);

		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = Sparky::Texture2D::Create("assets/textures/Checkerboard.png");
		m_LinuxLogo = Sparky::Texture2D::Create("assets/textures/Crate.png");

		std::dynamic_pointer_cast<Sparky::OpenGLShader>(textureShader)->Enable();
		std::dynamic_pointer_cast<Sparky::OpenGLShader>(textureShader)->SetUniform("u_Texture", 0);
	}

	~ExampleLayer() override
	{
		
	}

	void OnUpdate(Sparky::TimeStep ts) override
	{
		if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
			Sparky::Application::Get().CloseApplication();

		m_CameraController.OnUpdate(ts);

		Sparky::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
		Sparky::RenderCommand::Clear();

		Sparky::Renderer::BeginScene(m_CameraController.GetCamera());

		static Sparky::Math::mat4 scale = Sparky::Math::Scale(Sparky::Math::Identity(), Sparky::Math::vec3(0.1f));

		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_FlatColorShader)->Enable();
		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_FlatColorShader)->SetUniform("u_Color", m_SquareColor);

		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				Sparky::Math::vec3 position((float)x * 0.11f, (float)y * 0.11f, 0.0f);
				Sparky::Math::mat4 transform = Sparky::Math::Translate(Sparky::Math::Identity(), position) * scale;
				Sparky::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		Sparky::Renderer::Submit(textureShader, m_SquareVA, Sparky::Math::Scale(Sparky::Math::Identity(), Sparky::Math::vec3(1.5f)));
		
		m_LinuxLogo->Bind();
		Sparky::Renderer::Submit(textureShader, m_SquareVA, Sparky::Math::Scale(Sparky::Math::Identity(), Sparky::Math::vec3(1.5f)));

		///Triangle
		// Sparky::Renderer::Submit(m_TriangleShader, m_TriangleVA);

		Sparky::Renderer::EndScene();
	}

	void OnGuiRender() override
	{
		Gui::Begin("Settings");
		Gui::ColorEdit3("Square Color", Sparky::Math::ValuePtr(m_SquareColor));
		Gui::End();
	}

	void OnEvent(Sparky::Event& event) override
	{
		m_CameraController.OnEvent(event);
	}

private:
	Sparky::ShaderLibrary m_ShaderLibrary;

	Sparky::SharedRef<Sparky::VertexArray> m_TriangleVA;
	Sparky::SharedRef<Sparky::Shader> m_TriangleShader;

	Sparky::SharedRef<Sparky::VertexArray> m_SquareVA;
	Sparky::SharedRef<Sparky::Shader> m_FlatColorShader;

	Sparky::SharedRef<Sparky::Texture2D> m_Texture, m_LinuxLogo;

	Sparky::OrthographicCameraController m_CameraController;

	Sparky::Math::vec3 m_SquareColor{ 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Sparky::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() override
	{

	}
};

Sparky::Application* Sparky::CreateApplication() {
	return new Sandbox();
}