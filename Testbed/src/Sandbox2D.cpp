#include "Sandbox2D.h"

#include <Platform/OpenGL/OpenGLShader.h>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	m_SquareVA = Sparky::VertexArray::Create();

	float squareVertices[4 * 3] = {
		//position         
		 -0.5f, -0.5f, 0.0f,// bottom left
		  0.5f, -0.5f, 0.0f,// bottom right
		  0.5f,  0.5f, 0.0f,// top right
		 -0.5f,  0.5f, 0.0f,// top left
	};

	Sparky::SharedRef<Sparky::VertexBuffer> pSquareVB;
	pSquareVB = Sparky::VertexBuffer::Create(squareVertices, sizeof(squareVertices));

	pSquareVB->SetLayout({
		{ Sparky::ShaderDataType::Float3, "a_Position" },
	});

	m_SquareVA->AddVertexBuffer(pSquareVB);

	uint32_t squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
	Sparky::SharedRef<Sparky::IndexBuffer> pSquareIB = Sparky::IndexBuffer::Create(squareIndices, SP_ARRAYCOUNT(squareIndices));
	m_SquareVA->SetIndexBuffer(pSquareIB);

	m_FlatColorShader = Sparky::Shader::Create("assets/shaders/FlatColor.glsl");
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Sparky::TimeStep ts)
{
	// Update
	if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
		Sparky::Application::Get().CloseApplication();

	m_CameraController.OnUpdate(ts);

	// Render
	Sparky::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f });
	Sparky::RenderCommand::Clear();

	Sparky::Renderer::BeginScene(m_CameraController.GetCamera());

	static Sparky::Math::mat4 scale = Sparky::Math::Scale(Sparky::Math::Identity(), Sparky::Math::vec3(0.1f));

	std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_FlatColorShader)->Enable();
	std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_FlatColorShader)->SetUniform("u_Color", m_SquareColor);

	Sparky::Renderer::Submit(m_FlatColorShader, m_SquareVA, Sparky::Math::Scale(Sparky::Math::Identity(), Sparky::Math::vec3(1.5f)));

	Sparky::Renderer::EndScene();
}

void Sandbox2D::OnGuiRender()
{
	Gui::Begin("Settings");
	Gui::ColorEdit4("Square Color", Sparky::Math::ValuePtr(m_SquareColor));
	Gui::End();
}

void Sandbox2D::OnEvent(Sparky::Event& e)
{
	m_CameraController.OnEvent(e);
}
