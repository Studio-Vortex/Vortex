#include "sppch.h"
#include "Renderer2D.h"

#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky {

	struct Renderer2DState
	{
		SharedRef<Texture2D> WhiteTexture;
		SharedRef<Shader> TextureShader;

		SharedRef<VertexArray> QuadVertexArray;
		SharedRef<VertexBuffer> QuadVertexBuffer;
		SharedRef<IndexBuffer> QuadIndexBuffer;
	};

	static Renderer2DState* s_Data = nullptr;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DState();

		s_Data->QuadVertexArray = VertexArray::Create();

		float squareVertices[4 * 5] = {
			//position           tex coord
			 -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,// bottom left
			  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,// bottom right
			  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,// top right
			 -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,// top left
		};

		s_Data->QuadVertexBuffer = VertexBuffer::Create(squareVertices, sizeof(squareVertices));

		s_Data->QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		});

		s_Data->QuadVertexArray->AddVertexBuffer(s_Data->QuadVertexBuffer);

		uint32_t squareIndices[6] = { 0, 1, 2, 0, 2, 3 };
		s_Data->QuadIndexBuffer = IndexBuffer::Create(squareIndices, SP_ARRAYCOUNT(squareIndices));
		s_Data->QuadVertexArray->SetIndexBuffer(s_Data->QuadIndexBuffer);

		s_Data->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_Data->TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data->TextureShader->Enable();
		s_Data->TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_Data->TextureShader->Enable();
		s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
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
		s_Data->TextureShader->SetFloat4("u_Color", color);
		s_Data->WhiteTexture->Bind();

		auto transform = Math::Translate(Math::Rotate(Math::Scale(
			Math::Identity(), { size.x, size.y, 1.0f }), rotation, { 0.0f, 0.0f, 1.0f }), position
		);

		s_Data->TextureShader->SetMat4("u_Transform", transform);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color, float rotation)
	{
		DrawQuad(position, size, ColorToVec4(color), rotation);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color, float rotation)
	{
		DrawQuad(position, size, ColorToVec4(color), rotation);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture>& texture, float rotation, uint32_t scale, const Math::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, rotation, scale, color);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture>& texture, float rotation, uint32_t scale, const Math::vec4& color)
	{
		s_Data->TextureShader->SetFloat4("u_Color", color);
		s_Data->TextureShader->SetInt("u_TexScale", (int)scale);

		texture->Bind();

		auto transform = Math::Translate(Math::Rotate(Math::Scale(
			Math::Identity(), { size.x, size.y, 1.0f }), rotation, { 0.0f, 0.0f, 1.0f }), position
		);

		s_Data->TextureShader->SetMat4("u_Transform", transform);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture>& texture, Color color, float rotation, uint32_t scale)
	{
		DrawQuad(position, size, texture, rotation, scale, ColorToVec4(color));
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture>& texture, Color color, float rotation, uint32_t scale)
	{
		DrawQuad(position, size, texture, rotation, scale, ColorToVec4(color));
	}

}