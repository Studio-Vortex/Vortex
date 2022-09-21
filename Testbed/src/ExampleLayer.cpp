#include "ExampleLayer.h"

ExampleLayer::ExampleLayer()
	: Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
{
	m_TriangleVA = Sparky::VertexArray::Create();

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
	Sparky::SharedRef<Sparky::IndexBuffer> pTriangleIB = Sparky::IndexBuffer::Create(indices, SP_ARRAYCOUNT(indices));
	m_TriangleVA->SetIndexBuffer(pTriangleIB);

	m_SquareVA = Sparky::VertexArray::Create();

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
	Sparky::SharedRef<Sparky::IndexBuffer> pSquareIB = Sparky::IndexBuffer::Create(squareIndices, SP_ARRAYCOUNT(squareIndices));
	m_SquareVA->SetIndexBuffer(pSquareIB);

	m_TriangleShader = m_ShaderLibrary.Load("assets/shaders/VertexPosColor.glsl");
	m_FlatColorShader = m_ShaderLibrary.Load("assets/shaders/FlatColor.glsl");
	auto textureShader = m_ShaderLibrary.Load("assets/shaders/Renderer2D_Quad.glsl");

	m_Texture = Sparky::Texture2D::Create("assets/textures/Checkerboard.png");
	m_LinuxLogo = Sparky::Texture2D::Create("assets/textures/AwesomeFace.png");

	textureShader->Enable();
	textureShader->SetInt("u_Texture", 0);
}

void ExampleLayer::OnUpdate(Sparky::TimeStep delta)
{
	// Update
	if (Sparky::Input::IsKeyPressed(Sparky::Key::Escape))
		Sparky::Application::Get().Close();

	m_CameraController.OnUpdate(delta);

	// Render
	Sparky::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
	Sparky::RenderCommand::Clear();

	Sparky::Renderer::BeginScene(m_CameraController.GetCamera());

	static Math::mat4 scale = Math::Scale(Math::vec3(0.1f));

	m_FlatColorShader->Enable();
	m_FlatColorShader->SetFloat3("u_Color", m_SquareColor);

	for (int y = 0; y < 20; y++)
	{
		for (int x = 0; x < 20; x++)
		{
			Math::vec3 position((float)x * 0.11f, (float)y * 0.11f, 0.0f);
			Math::mat4 transform = Math::Translate(position) * scale;
			Sparky::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
		}
	}

	auto textureShader = m_ShaderLibrary.Get("Texture");

	m_Texture->Bind();
	Sparky::Renderer::Submit(textureShader, m_SquareVA, Math::Scale(Math::vec3(1.5f)));

	m_LinuxLogo->Bind();
	Sparky::Renderer::Submit(textureShader, m_SquareVA, Math::Scale(Math::vec3(1.5f)));

	///Triangle
	// Sparky::Renderer::Submit(m_TriangleShader, m_TriangleVA);

	Sparky::Renderer::EndScene();
}

void ExampleLayer::OnGuiRender()
{
	Gui::Begin("Settings");
	Gui::ColorEdit3("Square Color", Math::ValuePtr(m_SquareColor));
	Gui::End();
}

void ExampleLayer::OnEvent(Sparky::Event& event)
{
	m_CameraController.OnEvent(event);
}