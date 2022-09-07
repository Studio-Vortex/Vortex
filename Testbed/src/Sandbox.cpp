#include <Sparky.h>

class ExampleLayer : public Sparky::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(m_Camera.GetPosition())
	{
		m_TriangleVA.reset(Sparky::VertexArray::Create());

		float vertices[3 * 7] = {
			//position           color
			 -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			  0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		};

		std::shared_ptr<Sparky::VertexBuffer> pTriangleVB;
		pTriangleVB.reset(Sparky::VertexBuffer::Create(vertices, sizeof(vertices)));

		Sparky::BufferLayout layout = {
			{ Sparky::ShaderDataType::Float3, "a_Position" },
			{ Sparky::ShaderDataType::Float4, "a_Color" },
		};

		pTriangleVB->SetLayout(layout);
		m_TriangleVA->AddVertexBuffer(pTriangleVB);

		uint32_t indices[3] = { 0, 1, 2 };
		std::shared_ptr<Sparky::IndexBuffer> pTriangleIB;
		pTriangleIB.reset(Sparky::IndexBuffer::Create(indices, SP_ARRAYCOUNT(indices)));
		m_TriangleVA->SetIndexBuffer(pTriangleIB);

		m_SquareVA.reset(Sparky::VertexArray::Create());

		float squareVertices[4 * 7] = {
			//position
			 -0.75f, -0.75f, 0.0f, // bottom left
			  0.75f, -0.75f, 0.0f, // bottom right
			  0.75f,  0.75f, 0.0f, // top right
			 -0.75f,  0.75f, 0.0f, // top left
		};

		std::shared_ptr<Sparky::VertexBuffer> pSquareVB;
		pSquareVB.reset(Sparky::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		pSquareVB->SetLayout({
			{ Sparky::ShaderDataType::Float3, "a_Position" },
		});
		m_SquareVA->AddVertexBuffer(pSquareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
		std::shared_ptr<Sparky::IndexBuffer> pSquareIB;
		pSquareIB.reset(Sparky::IndexBuffer::Create(squareIndices, SP_ARRAYCOUNT(squareIndices)));
		m_SquareVA->SetIndexBuffer(pSquareIB);

		std::string vertexSrc = R"(
			#version 460 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec4 f_Color;

			uniform mat4 u_ViewProjection;

			void main()
			{
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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

		m_TriangleShader.reset(new Sparky::Shader(vertexSrc, fragmentSrc));

		std::string vertexSrc2 = R"(
			#version 460 core

			layout(location = 0) in vec3 a_Position;

			out vec3 f_Position;

			uniform mat4 u_ViewProjection;

			void main()
			{
				f_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
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

		m_SquareShader.reset(new Sparky::Shader(vertexSrc2, fragmentSrc2));
	}

	~ExampleLayer() override
	{
		
	}

	void ProcessInput(Sparky::TimeStep ts)
	{
		float deltaTime = ts;

		if (Sparky::Input::IsKeyPressed(SP_KEY_W))
			m_CameraPosition.y += m_CameraMoveSpeed * deltaTime;
		else if (Sparky::Input::IsKeyPressed(SP_KEY_S))
			m_CameraPosition.y -= m_CameraMoveSpeed * deltaTime;

		if (Sparky::Input::IsKeyPressed(SP_KEY_A))
			m_CameraPosition.x -= m_CameraMoveSpeed * deltaTime;
		else if (Sparky::Input::IsKeyPressed(SP_KEY_D))
			m_CameraPosition.x += m_CameraMoveSpeed * deltaTime;

		if (Sparky::Input::IsKeyPressed(SP_KEY_R))
			m_CameraRotation -= m_CameraRotationSpeed * deltaTime;
		else if (Sparky::Input::IsKeyPressed(SP_KEY_T))
			m_CameraRotation += m_CameraRotationSpeed * deltaTime;

		if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
			Sparky::Application::Get().CloseApplication();
	}

	void OnUpdate(Sparky::TimeStep ts) override
	{
		SP_CORE_INFO("Delta Time: {}s ({}ms)", ts.GetDeltaTime(), ts.GetDeltaTimeMs());
		ProcessInput(ts);

		Sparky::RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.8f });
		Sparky::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Sparky::Renderer::BeginScene(m_Camera);

		Sparky::Renderer::Submit(m_SquareShader, m_SquareVA);
		Sparky::Renderer::Submit(m_TriangleShader, m_TriangleVA);

		Sparky::Renderer::EndScene();
	}

	void OnGuiRender() override
	{

	}

	void OnEvent(Sparky::Event& event) override
	{

	}

private:
	std::shared_ptr<Sparky::VertexArray> m_TriangleVA;
	std::shared_ptr<Sparky::Shader> m_TriangleShader;

	std::shared_ptr<Sparky::VertexArray> m_SquareVA;
	std::shared_ptr<Sparky::Shader> m_SquareShader;

	Sparky::OrthographicCamera m_Camera;
	Sparky::Math::vec3 m_CameraPosition;
	float m_CameraRotation = 0.0f;

	float m_CameraMoveSpeed = -1.0f;
	float m_CameraRotationSpeed = 30.0f;
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