#include "sppch.h"
#include "Renderer2D.h"

#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky
{

	struct QuadVertex
	{
		Math::vec3 Position;
		Math::vec4 Color;
		Math::vec2 TexCoord;
		float TexIndex;
		float TexScale;
	};

	struct Renderer2DData
	{
		static constexpr inline uint32_t MaxQuads = 20'000;
		static constexpr inline uint32_t MaxVertices = MaxQuads * VERTICES_PER_QUAD;
		static constexpr inline uint32_t MaxIndices = MaxQuads * INDICES_PER_QUAD;
		static constexpr inline uint32_t MaxTextureSlots = 32; // TODO: RendererCapabilities

		SharedRef<VertexArray> QuadVA;
		SharedRef<VertexBuffer> QuadVB;
		SharedRef<Shader> TextureShader;
		SharedRef<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<SharedRef<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture

		Math::vec4 QuadVertexPositions[4];

		Renderer2D::Statistics Stats;
	};

	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		SP_PROFILE_FUNCTION();

		s_Data.QuadVA = VertexArray::Create();

		s_Data.QuadVB = VertexBuffer::Create(Renderer2DData::MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float,  "a_TexIndex" },
			{ ShaderDataType::Float,  "a_TexScale" },
		});

		s_Data.QuadVA->AddVertexBuffer(s_Data.QuadVB);

		s_Data.QuadVertexBufferBase = new QuadVertex[Renderer2DData::MaxVertices];

		uint32_t* quadIndices = new uint32_t[Renderer2DData::MaxIndices];

		uint32_t offset = 0;

		for (uint32_t i = 0; i < Renderer2DData::MaxIndices; i += INDICES_PER_QUAD)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += VERTICES_PER_QUAD;
		}

		SharedRef<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, Renderer2DData::MaxIndices);
		s_Data.QuadVA->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[Renderer2DData::MaxTextureSlots];

		for (uint32_t i = 0; i < Renderer2DData::MaxTextureSlots; i++)
			samplers[i] = i;

		s_Data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		s_Data.TextureShader->Enable();
		// Set the sampler2D array on the GPU
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, Renderer2DData::MaxTextureSlots);

		// Set the first texture slot to out default white texture
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		// Create a quad's set of vertices at the origin
		s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

#if SP_RENDERER_STATISTICS
		ResetStats();
#endif // SP_RENDERER_STATISTICS
	}

	void Renderer2D::Shutdown()
	{
		SP_PROFILE_FUNCTION();

		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const Camera& camera, const Math::mat4& transform)
	{
		SP_PROFILE_FUNCTION();

		Math::mat4 viewProjection = camera.GetProjection() * Math::Inverse(transform);

		s_Data.TextureShader->Enable();
		s_Data.TextureShader->SetMat4("u_ViewProjection", viewProjection);

		// Set the index count to 0 for the new batch
		s_Data.QuadIndexCount = 0;
		// Set the pointer to the beggining of the vertex buffer
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		// Reset the starting frame texture slot past the White Texture
		s_Data.TextureSlotIndex = 1;
	}

    void Renderer2D::BeginScene(const EditorCamera& camera)
    {
		SP_PROFILE_FUNCTION();

		Math::mat4 viewProjection = camera.GetViewProjection();

		s_Data.TextureShader->Enable();
		s_Data.TextureShader->SetMat4("u_ViewProjection", viewProjection);

		StartBatch();
    }

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		SP_PROFILE_FUNCTION();

		s_Data.TextureShader->Enable();
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		StartBatch();
	}

	void Renderer2D::StartBatch()
	{
		// Set the index count to 0 for the new batch
		s_Data.QuadIndexCount = 0;
		// Set the pointer to the beggining of the vertex buffer
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		// Reset the starting frame texture slot past the White Texture
		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();

		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		SP_PROFILE_FUNCTION();

		CopyDataToBuffer();

		// Render vertices as a batch
		Flush();
	}

	void Renderer2D::CopyDataToBuffer()
	{
		// Calculate the size of the vertex buffer
		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
		// Copy data to GPU buffer
		s_Data.QuadVB->SetData(s_Data.QuadVertexBufferBase, dataSize);
	}

	void Renderer2D::Flush()
	{
		// Bind all textures that were used in queue
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		RenderCommand::DrawIndexed(s_Data.QuadVA, s_Data.QuadIndexCount);
		s_Data.Stats.DrawCalls++;
	}

	void Renderer2D::AddToVertexBuffer(const Math::mat4& transform, const Math::vec4& color, const Math::vec2* textureCoords, float textureIndex, float textureScale)
	{
		for (uint32_t i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TexScale = textureScale;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += INDICES_PER_QUAD;

#if SP_RENDERER_STATISTICS
		s_Data.Stats.QuadCount++;
#endif // SP_RENDERER_STATISTICS
	}

    void Renderer2D::DrawQuad(const Math::mat4& transform, const Math::vec3& color)
    {
		DrawQuad(transform, { color.r, color.g, color.b, 1.0f });
    }

    void Renderer2D::DrawQuad(const Math::mat4& transform, const Math::vec4& color)
    {
		SP_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		constexpr float textureIndex = 0.0f; // Our White Texture
		constexpr float textureScale = 1.0f;

		Math::vec2 textureCoords[4] = { {0.0f, 0.0f}, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		AddToVertexBuffer(transform, color, textureCoords, textureIndex, textureScale);
    }

	void Renderer2D::DrawQuad(const Math::mat4& transform, Color color)
	{
		DrawQuad(transform, ColorToVec4(color));
	}

	void Renderer2D::DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec3& tintColor)
	{
		DrawQuad(transform, texture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor)
	{
		SP_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;

		// Find the texture ID for the given texture so we can give it to the vertex descriptions
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		Math::vec2 textureCoords[4] = { {0.0f, 0.0f}, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		// Vertex Descriptions
		AddToVertexBuffer(transform, tintColor, textureCoords, textureIndex, scale);
	}

	void Renderer2D::DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, Color tintColor)
	{
		DrawQuad(transform, texture, scale, ColorToVec4(tintColor));
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
		Math::mat4 transform = Math::Translate(position) * Math::Scale({ size.x, size.y, 1.0f });
		DrawQuad(transform, color);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color)
	{
		DrawQuad(position, size, ColorToVec4(color));
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor)
	{
		Math::mat4 transform = Math::Translate(position) * Math::Scale({ size.x, size.y, 1.0f });
		DrawQuad(transform, texture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, float scale, Color tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, float scale, Color tintColor)
	{
		DrawQuad(position, size, texture, scale, ColorToVec4(tintColor));
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec3& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec4& tintColor)
	{
		SP_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;

		const SharedRef<Texture2D>& texture = subtexture->GetTexure();

		// Find the texture ID for the given texture so we can give it to the vertex descriptions
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		Math::mat4 transform = Math::Translate(position) * Math::Scale({ size.x, size.y, 1.0f });
		const Math::vec2* textureCoords = subtexture->GetTextureCoords();

		// Vertex Descriptions
		AddToVertexBuffer(transform, tintColor, textureCoords, textureIndex, scale);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, Color tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, float scale, Color tintColor)
	{
		DrawQuad(position, size, subtexture, scale, ColorToVec4(tintColor));
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const Math::vec3& color)
	{
		DrawRotatedQuad(transform, { color.r, color.g, color.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const Math::vec4& color)
	{
		SP_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		static constexpr float textureIndex = 0.0f; // Our White Texture
		static constexpr float textureScale = 1.0f;

		Math::vec2 textureCoords[4] = { {0.0f, 0.0f}, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		AddToVertexBuffer(transform, color, textureCoords, textureIndex, textureScale);
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, Color color)
	{
		DrawRotatedQuad(transform, ColorToVec4(color));
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad(transform, texture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor)
	{
		SP_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		Math::vec2 textureCoords[4] = { {0.0f, 0.0f}, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		AddToVertexBuffer(transform, tintColor, textureCoords, textureIndex, scale);
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, float scale, Color tintColor)
	{
		DrawRotatedQuad(transform, texture, scale, ColorToVec4(tintColor));
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
		Math::mat4 transform = Math::Translate(position) * Math::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Math::Scale({ size.x, size.y, 1.0f });
		DrawRotatedQuad(transform, color);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, Color color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, Color color)
	{
		DrawRotatedQuad(position, size, rotation, ColorToVec4(color));
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad(position, size, rotation, texture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, const Math::vec4& tintColor)
	{
		Math::mat4 transform = Math::Translate(position) * Math::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Math::Scale({ size.x, size.y, 1.0f });
		DrawRotatedQuad(transform, texture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subtexture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad(position, size, rotation, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, const Math::vec4& tintColor)
	{
		SP_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;

		const SharedRef<Texture2D>& texture = subtexture->GetTexure();

		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		Math::mat4 transform = Math::Translate(position) * Math::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Math::Scale({ size.x, size.y, 1.0f });
		const Math::vec2* textureCoords = subtexture->GetTextureCoords();

		AddToVertexBuffer(transform, tintColor, textureCoords, textureIndex, scale);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, Color tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, float scale, Color tintColor)
	{
		DrawRotatedQuad(position, size, rotation, texture, scale, ColorToVec4(tintColor));
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, Color tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, float scale, Color tintColor)
	{
		DrawRotatedQuad(position, size, rotation, subtexture, scale, ColorToVec4(tintColor));
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, NULL, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}

}