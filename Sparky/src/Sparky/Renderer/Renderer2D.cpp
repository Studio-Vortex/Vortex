#include "sppch.h"
#include "Renderer2D.h"

#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky {

	constexpr uint32_t SP_HEX_WHITE = 0xffffffff;

	struct QuadVertex
	{
		Math::vec3 Position;
		Math::vec4 Color;
		Math::vec2 TexCoord;
		// TODO: color, texid
	};

	constexpr uint32_t SP_VERTICES_PER_QUAD = 4;
	constexpr uint32_t SP_INDICES_PER_QUAD = 6;

	struct Renderer2DData
	{
		const uint32_t MaxQuads = 10'000;
		const uint32_t MaxVertices = MaxQuads * SP_VERTICES_PER_QUAD;
		const uint32_t MaxIndices = MaxQuads * SP_INDICES_PER_QUAD;

		SharedRef<VertexArray> QuadVA;
		SharedRef<VertexBuffer> QuadVB;
		SharedRef<Shader> TextureShader;
		SharedRef<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
	};

	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		SP_PROFILE_FUNCTION();

		s_Data.QuadVA = VertexArray::Create();

		s_Data.QuadVB = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));

		s_Data.QuadVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		});

		s_Data.QuadVA->AddVertexBuffer(s_Data.QuadVB);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;

		for (uint32_t i = 0; i < s_Data.MaxIndices; i += SP_INDICES_PER_QUAD)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += SP_VERTICES_PER_QUAD;
		}

		SharedRef<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.QuadVA->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = SP_HEX_WHITE;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		s_Data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data.TextureShader->Enable();
		s_Data.TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		SP_PROFILE_FUNCTION();

		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		SP_PROFILE_FUNCTION();

		s_Data.TextureShader->Enable();
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::EndScene()
	{
		SP_PROFILE_FUNCTION();

		uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
		s_Data.QuadVB->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush()
	{
		RenderCommand::DrawIndexed(s_Data.QuadVA, s_Data.QuadIndexCount);
	}
	
	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec3& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, { color.r, color.g, color.b, 1.0f });
	}
	
	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const Math::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec3& color)
	{
		DrawQuad(position, size, { color.r, color.g, color.b, 1.0f });
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color)
	{
		SP_PROFILE_FUNCTION();

		s_Data.QuadVertexBufferPtr->Position = position;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = { position.x, position.y + size.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += SP_INDICES_PER_QUAD;

		/*s_Data.TextureShader->SetFloat4("u_Color", color);
		s_Data.WhiteTexture->Bind();

		auto transform = Math::Translate(Math::Identity(), position) * Math::Scale(Math::Identity(), { size.x, size.y, 1.0f });
		s_Data.TextureShader->SetMat4("u_Transform", transform);

		s_Data.QuadVA->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVA);*/
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, ColorToVec4(color));
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color)
	{
		DrawQuad(position, size, ColorToVec4(color));
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, uint32_t scale, const SharedRef<Texture>& texture, const Math::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, scale, texture, color);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, uint32_t scale, const SharedRef<Texture>& texture, const Math::vec4& color)
	{
		SP_PROFILE_FUNCTION();

		s_Data.TextureShader->SetFloat4("u_Color", color);
		s_Data.TextureShader->SetInt("u_TexScale", (int)scale);

		texture->Bind();

		auto transform = Math::Translate(Math::Identity(), position) * Math::Scale(Math::Identity(), { size.x, size.y, 1.0f });
		s_Data.TextureShader->SetMat4("u_Transform", transform);

		s_Data.QuadVA->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVA);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, uint32_t scale, const SharedRef<Texture>& texture, Color color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, scale, texture, ColorToVec4(color));
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, uint32_t scale, const SharedRef<Texture>& texture, Color color)
	{
		DrawQuad(position, size, scale, texture, ColorToVec4(color));
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec3& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, { color.r, color.g, color.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec3& color)
	{
		DrawRotatedQuad(position, size, rotation, { color.r, color.g, color.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec4& color)
	{
		SP_PROFILE_FUNCTION();

		s_Data.TextureShader->SetFloat4("u_Color", color);
		s_Data.TextureShader->SetInt("u_TexScale", 1);

		auto transform = Math::Scale(Math::Rotate(Math::Translate(Math::Identity(), position), rotation, { 0.0f, 0.0f, 1.0f }), { size.x, size.y, 1.0f });
		s_Data.TextureShader->SetMat4("u_Transform", transform);

		s_Data.QuadVA->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVA);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, Color color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, ColorToVec4(color));
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, Color color)
	{
		DrawRotatedQuad(position, size, rotation, ColorToVec4(color));
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, uint32_t scale, const SharedRef<Texture>& texture, const Math::vec3& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, scale, texture, { color.r, color.g, color.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, uint32_t scale, const SharedRef<Texture>& texture, const Math::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, scale, texture, color);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, uint32_t scale, const SharedRef<Texture>& texture, const Math::vec3& color)
	{
		DrawRotatedQuad(position, size, rotation, scale, texture, { color.r, color.g, color.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, uint32_t scale, const SharedRef<Texture>& texture, const Math::vec4& color)
	{
		SP_PROFILE_FUNCTION();

		s_Data.TextureShader->SetFloat4("u_Color", color);
		s_Data.TextureShader->SetInt("u_TexScale", (int)scale);

		texture->Bind();

		auto transform = Math::Scale(Math::Rotate(Math::Translate(Math::Identity(), position), rotation, { 0.0f, 0.0f, 1.0f }), { size.x, size.y, 1.0f });
		s_Data.TextureShader->SetMat4("u_Transform", transform);

		s_Data.QuadVA->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVA);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, uint32_t scale, const SharedRef<Texture>& texture, Color color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, scale, texture, color);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, uint32_t scale, const SharedRef<Texture>& texture, Color color)
	{
		DrawRotatedQuad(position, size, rotation, scale, texture, ColorToVec4(color));
	}

}