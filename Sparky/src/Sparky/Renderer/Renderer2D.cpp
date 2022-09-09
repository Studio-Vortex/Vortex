#include "sppch.h"
#include "Renderer2D.h"

#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky {

	struct Renderer2DState
	{
		SharedRef<VertexArray> QuadVertexArray;
		SharedRef<Shader> FlatColorShader;
		SharedRef<VertexBuffer> QuadVertexBuffer;
		SharedRef<IndexBuffer> QuadIndexBuffer;
	};

	static Renderer2DState* s_Data = nullptr;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DState();

		s_Data->QuadVertexArray = VertexArray::Create();

		float squareVertices[4 * 3] = {
			//position         
			 -0.5f, -0.5f, 0.0f,// bottom left
			  0.5f, -0.5f, 0.0f,// bottom right
			  0.5f,  0.5f, 0.0f,// top right
			 -0.5f,  0.5f, 0.0f,// top left
		};

		s_Data->QuadVertexBuffer = VertexBuffer::Create(squareVertices, sizeof(squareVertices));

		s_Data->QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
		});

		s_Data->QuadVertexArray->AddVertexBuffer(s_Data->QuadVertexBuffer);

		uint32_t squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
		s_Data->QuadIndexBuffer = IndexBuffer::Create(squareIndices, SP_ARRAYCOUNT(squareIndices));
		s_Data->QuadVertexArray->SetIndexBuffer(s_Data->QuadIndexBuffer);

		s_Data->FlatColorShader = Shader::Create("assets/shaders/FlatColor.glsl");
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_Data->FlatColorShader->Enable();
		s_Data->FlatColorShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene()
	{

	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec3& color, float rotation)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, { color.r, color.g, color.b, 1.0f }, rotation);
	}
	
	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec4& color, float rotation)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color, rotation);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec3& color, float rotation)
	{
		DrawQuad(position, size, { color.r, color.g, color.b, 1.0f }, rotation);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color, float rotation)
	{
		s_Data->FlatColorShader->Enable();
		s_Data->FlatColorShader->SetFloat4("u_Color", color);

		auto transform = Math::Translate(Math::Rotate(Math::Scale(
			Math::Identity(), { size.x, size.y, 1.0f }), rotation, { 0.0f, 0.0f, 1.0f }),position
		);

		s_Data->FlatColorShader->SetMat4("u_Transform", transform);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

}