#include "vxpch.h"
#include "Renderer2D.h"

#include "Vortex/Renderer/RenderCommand.h"
#include "Vortex/Renderer/VertexArray.h"
#include "Vortex/Renderer/Shader.h"

#include "Vortex/Renderer/Font/MSDFData.h"

#include <codecvt>

namespace Vortex
{
	static constexpr const char* QUAD_SHADER_PATH   = "Resources/Shaders/Renderer2D_Quad.glsl";
	static constexpr const char* CIRCLE_SHADER_PATH = "Resources/Shaders/Renderer2D_Circle.glsl";
	static constexpr const char* LINE_SHADER_PATH   = "Resources/Shaders/Renderer2D_Line.glsl";
	static constexpr const char* TEXT_SHADER_PATH = "Resources/Shaders/Renderer2D_Text.glsl";

	struct QuadVertex
	{
		Math::vec3 Position;
		Math::vec4 Color;
		Math::vec2 TexCoord;
		float TexIndex;
		Math::vec2 TexScale;

		// Editor-only
		int EntityID;
	};

	struct CircleVertex
	{
		Math::vec3 WorldPosition;
		Math::vec3 LocalPosition;
		Math::vec4 Color;
		float Thickness;
		float Fade;

		// Editor-only
		int EntityID;
	};

	struct LineVertex
	{
		Math::vec3 Position;
		Math::vec4 Color;

		// Editor-only
		int EntityID;
	};

	struct TextVertex
	{
		Math::vec3 Position;
		Math::vec4 Color;
		Math::vec2 TexCoord;
		float TexIndex;

		// Editor-only
		int EntityID;
	};

	struct Renderer2DInternalData
	{
		static constexpr inline uint32_t MaxQuads = 20'000;
		static constexpr inline uint32_t MaxVertices = MaxQuads * VERTICES_PER_QUAD;
		static constexpr inline uint32_t MaxIndices = MaxQuads * INDICES_PER_QUAD;
		static constexpr inline uint32_t MaxTextureSlots = 32; // TODO: RendererCapabilities

		SharedRef<Texture2D> WhiteTexture; // Default texture

		SharedRef<ShaderLibrary> ShaderLibrary = nullptr;

		SharedRef<VertexArray> UnitQuadVA = nullptr;
		SharedRef<VertexBuffer> UnitQuadVB = nullptr;
		SharedRef<IndexBuffer> UnitQuadIB = nullptr;

		SharedRef<VertexArray> QuadVA = nullptr;
		SharedRef<VertexBuffer> QuadVB = nullptr;

		SharedRef<VertexArray> CircleVA = nullptr;
		SharedRef<VertexBuffer> CircleVB = nullptr;

		SharedRef<VertexArray> LineVA = nullptr;
		SharedRef<VertexBuffer> LineVB = nullptr;

		SharedRef<VertexArray> TextVA = nullptr;
		SharedRef<VertexBuffer> TextVB = nullptr;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		uint32_t CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;

		uint32_t LineVertexCount = 0;
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;

		uint32_t TextIndexCount = 0;
		TextVertex* TextVertexBufferBase = nullptr;
		TextVertex* TextVertexBufferPtr = nullptr;

		float LineWidth = 1.5f;

		std::array<SharedRef<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture

		std::array<SharedRef<Texture2D>, MaxTextureSlots> FontTextureSlots;
		uint32_t FontTextureSlotIndex = 0;

		Math::vec4 QuadVertexPositions[4];

		RenderStatistics Renderer2DStatistics;
		RendererAPI::TriangleCullMode CullMode = RendererAPI::TriangleCullMode::None;
	};

	static Renderer2DInternalData s_Data;

	void Renderer2D::Init(RendererAPI::TriangleCullMode cullMode)
	{
		VX_PROFILE_FUNCTION();

		float vertices[] = {
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		/// Unit Quad
		s_Data.UnitQuadVA = VertexArray::Create();
		s_Data.UnitQuadVB = VertexBuffer::Create(vertices, sizeof(float) * 20);
		s_Data.UnitQuadVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		s_Data.UnitQuadVA->AddVertexBuffer(s_Data.UnitQuadVB);

		/// Quads
		s_Data.QuadVA = VertexArray::Create();

		s_Data.QuadVB = VertexBuffer::Create(Renderer2DInternalData::MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float,  "a_TexIndex" },
			{ ShaderDataType::Float2, "a_TexScale" },
			{ ShaderDataType::Int,    "a_EntityID" },
		});

		s_Data.QuadVA->AddVertexBuffer(s_Data.QuadVB);
		s_Data.QuadVertexBufferBase = new QuadVertex[Renderer2DInternalData::MaxVertices];

		uint32_t* quadIndices = new uint32_t[Renderer2DInternalData::MaxIndices];

		uint32_t offset = 0;
		for (size_t i = 0; i < Renderer2DInternalData::MaxIndices; i += INDICES_PER_QUAD)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += VERTICES_PER_QUAD;
		}

		SharedRef<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, Renderer2DInternalData::MaxIndices);
		s_Data.QuadVA->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		/// Circles
		s_Data.CircleVA = VertexArray::Create();

		s_Data.CircleVB = VertexBuffer::Create(Renderer2DInternalData::MaxVertices * sizeof(CircleVertex));
		s_Data.CircleVB->SetLayout({
			{ ShaderDataType::Float3, "a_WorldPosition" },
			{ ShaderDataType::Float3, "a_LocalPosition" },
			{ ShaderDataType::Float4, "a_Color"         },
			{ ShaderDataType::Float,  "a_Thickness"     },
			{ ShaderDataType::Float,  "a_Fade"          },
			{ ShaderDataType::Int,    "a_EntityID"      },
		});

		s_Data.CircleVA->AddVertexBuffer(s_Data.CircleVB);
		s_Data.CircleVA->SetIndexBuffer(quadIB); // Use quad IB
		s_Data.CircleVertexBufferBase = new CircleVertex[Renderer2DInternalData::MaxVertices];

		/// Lines
		s_Data.LineVA = VertexArray::Create();

		s_Data.LineVB = VertexBuffer::Create(Renderer2DInternalData::MaxVertices * sizeof(LineVertex));
		s_Data.LineVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Int,    "a_EntityID" },
		});

		s_Data.LineVA->AddVertexBuffer(s_Data.LineVB);
		s_Data.LineVertexBufferBase = new LineVertex[Renderer2DInternalData::MaxVertices];

		s_Data.TextVA = VertexArray::Create();

		s_Data.TextVB = VertexBuffer::Create(Renderer2DInternalData::MaxVertices * sizeof(TextVertex));
		s_Data.TextVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float,  "a_TexIndex" },
			{ ShaderDataType::Int,    "a_EntityID" },
		});

		s_Data.TextVA->AddVertexBuffer(s_Data.TextVB);
		s_Data.TextVA->SetIndexBuffer(quadIB);
		s_Data.TextVertexBufferBase = new TextVertex[Renderer2DInternalData::MaxVertices];

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		// Gpu Sampler2D Array
		int32_t samplers[s_Data.MaxTextureSlots];
		for (size_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		// Set the first texture slot to out default white texture
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		s_Data.ShaderLibrary = ShaderLibrary::Create();

		SharedRef<Shader> quadShader = s_Data.ShaderLibrary->Load("Quad", QUAD_SHADER_PATH);
		// Set the sampler2D array on the GPU
		quadShader->SetIntArray("u_Textures", samplers, Renderer2DInternalData::MaxTextureSlots);

		s_Data.ShaderLibrary->Load("Circle", CIRCLE_SHADER_PATH);
		s_Data.ShaderLibrary->Load("Line", LINE_SHADER_PATH);
		SharedRef<Shader> textShader = s_Data.ShaderLibrary->Load("Text", TEXT_SHADER_PATH);
		textShader->Enable();
		textShader->SetIntArray("u_Textures", samplers, Renderer2DInternalData::MaxTextureSlots);

		// Create a quad's set of vertices at the origin in ndc
		s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		SetLineWidth(s_Data.LineWidth);
		SetCullMode(cullMode);

#if VX_RENDERER_STATISTICS
		ResetStats();
#endif // SP_RENDERER_STATISTICS
	}

	void Renderer2D::Shutdown()
	{
		VX_PROFILE_FUNCTION();

		delete[] s_Data.QuadVertexBufferBase;
		delete[] s_Data.CircleVertexBufferBase;
		delete[] s_Data.LineVertexBufferBase;
		delete[] s_Data.TextVertexBufferBase;
	}

	void Renderer2D::BeginScene(const Camera& camera, const Math::mat4& transform)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 viewProjection = camera.GetProjectionMatrix() * Math::Inverse(transform);

		SharedRef<Shader> quadShader = s_Data.ShaderLibrary->Get("Quad");
		quadShader->Enable();
		quadShader->SetMat4("u_ViewProjection", viewProjection);

		SharedRef<Shader> circleShader = s_Data.ShaderLibrary->Get("Circle");
		circleShader->Enable();
		circleShader->SetMat4("u_ViewProjection", viewProjection);

		SharedRef<Shader> lineShader = s_Data.ShaderLibrary->Get("Line");
		lineShader->Enable();
		lineShader->SetMat4("u_ViewProjection", viewProjection);

		SharedRef<Shader> textShader = s_Data.ShaderLibrary->Get("Text");
		textShader->Enable();
		textShader->SetMat4("u_ViewProjection", viewProjection);

		StartBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 viewProjection = camera->GetViewProjection();

		SharedRef<Shader> quadShader = s_Data.ShaderLibrary->Get("Quad");
		quadShader->Enable();
		quadShader->SetMat4("u_ViewProjection", viewProjection);

		SharedRef<Shader> circleShader = s_Data.ShaderLibrary->Get("Circle");
		circleShader->Enable();
		circleShader->SetMat4("u_ViewProjection", viewProjection);

		SharedRef<Shader> lineShader = s_Data.ShaderLibrary->Get("Line");
		lineShader->Enable();
		lineShader->SetMat4("u_ViewProjection", viewProjection);

		SharedRef<Shader> textShader = s_Data.ShaderLibrary->Get("Text");
		textShader->Enable();
		textShader->SetMat4("u_ViewProjection", viewProjection);

		StartBatch();
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 viewProjection = camera.GetViewProjectionMatrix();

		SharedRef<Shader> quadShader = s_Data.ShaderLibrary->Get("Quad");
		quadShader->Enable();
		quadShader->SetMat4("u_ViewProjection", viewProjection);

		SharedRef<Shader> circleShader = s_Data.ShaderLibrary->Get("Circle");
		circleShader->Enable();
		circleShader->SetMat4("u_ViewProjection", viewProjection);

		SharedRef<Shader> lineShader = s_Data.ShaderLibrary->Get("Line");
		lineShader->Enable();
		lineShader->SetMat4("u_ViewProjection", viewProjection);

		SharedRef<Shader> textShader = s_Data.ShaderLibrary->Get("Text");
		textShader->Enable();
		textShader->SetMat4("u_ViewProjection", viewProjection);

		StartBatch();
	}

	void Renderer2D::StartBatch()
	{
		// Set the index count to 0 for the new batch
		s_Data.QuadIndexCount = 0;
		// Set the pointer to the beginning of the vertex buffer
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		// Same for circles
		s_Data.CircleIndexCount = 0;
		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

		// Lines
		s_Data.LineVertexCount = 0;
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

		// Text
		s_Data.TextIndexCount = 0;
		s_Data.TextVertexBufferPtr = s_Data.TextVertexBufferBase;

		// Reset the texture slot index (0 is the default white texture)
		s_Data.TextureSlotIndex = 1;
		// Reset font texture slot
		s_Data.FontTextureSlotIndex = 0;
	}

	void Renderer2D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		VX_PROFILE_FUNCTION();

		// Render vertices as a batch
		Flush();
	}

	void Renderer2D::Flush()
	{
		/// Quads
		if (s_Data.QuadIndexCount)
		{
			// Calculate the size of the vertex buffer
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
			// Copy data to GPU buffer
			s_Data.QuadVB->SetData(s_Data.QuadVertexBufferBase, dataSize);

			// Bind all textures used in the batch
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);

			// Bind a shader and make a draw call
			s_Data.ShaderLibrary->Get("Quad")->Enable();
			RenderCommand::DrawIndexed(s_Data.QuadVA, s_Data.QuadIndexCount);
			s_Data.Renderer2DStatistics.DrawCalls++;
		}

		/// Circles
		if (s_Data.CircleIndexCount)
		{
			// Calculate the size of the vertex buffer
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
			// Copy data to GPU buffer
			s_Data.CircleVB->SetData(s_Data.CircleVertexBufferBase, dataSize);

			// Bind a shader and make a draw call
			s_Data.ShaderLibrary->Get("Circle")->Enable();
			RenderCommand::DrawIndexed(s_Data.CircleVA, s_Data.CircleIndexCount);
			s_Data.Renderer2DStatistics.DrawCalls++;
		}

		/// Lines
		if (s_Data.LineVertexCount)
		{
			// Calculate the size of the vertex buffer
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
			// Copy data to GPU buffer
			s_Data.LineVB->SetData(s_Data.LineVertexBufferBase, dataSize);

			// Bind a shader and make a draw call
			s_Data.ShaderLibrary->Get("Line")->Enable();
			RenderCommand::DrawLines(s_Data.LineVA, s_Data.LineVertexCount);
			s_Data.Renderer2DStatistics.DrawCalls++;
		}

		/// Text
		if (s_Data.TextIndexCount)
		{
			// Calculate the size of the vertex buffer
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.TextVertexBufferPtr - (uint8_t*)s_Data.TextVertexBufferBase);
			// Copy data to GPU buffer
			s_Data.TextVB->SetData(s_Data.TextVertexBufferBase, dataSize);

			// Bind all textures used in the batch
			for (uint32_t i = 0; i < s_Data.FontTextureSlotIndex; i++)
				s_Data.FontTextureSlots[i]->Bind(i);

			// Bind a shader and make a draw call
			s_Data.ShaderLibrary->Get("Text")->Enable();
			RenderCommand::DrawIndexed(s_Data.TextVA, s_Data.TextIndexCount);
			s_Data.Renderer2DStatistics.DrawCalls++;
		}
	}

	void Renderer2D::AddToQuadVertexBuffer(const Math::mat4& transform, const Math::vec4& color, const Math::vec2* textureCoords, float textureIndex, const Math::vec2& textureScale, int entityID)
	{
		for (size_t i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TexScale = textureScale;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += INDICES_PER_QUAD;

#if VX_RENDERER_STATISTICS
		s_Data.Renderer2DStatistics.QuadCount++;
#endif // SP_RENDERER_STATISTICS
	}

	void Renderer2D::AddToCircleVertexBuffer(const Math::mat4& transform, const Math::vec4& color, float thickness, float fade, int entityID)
	{
		for (size_t i = 0; i < 4; i++)
		{
			s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i]; // Use quad vertex positions
			s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f; // Multiply by 2 to get the coordinates into a -1 -> 1 space
			s_Data.CircleVertexBufferPtr->Color = color;
			s_Data.CircleVertexBufferPtr->Thickness = thickness;
			s_Data.CircleVertexBufferPtr->Fade = fade;
			s_Data.CircleVertexBufferPtr->EntityID = entityID;
			s_Data.CircleVertexBufferPtr++;
		}

		s_Data.CircleIndexCount += INDICES_PER_QUAD;

#if VX_RENDERER_STATISTICS
		s_Data.Renderer2DStatistics.QuadCount++;
#endif // SP_RENDERER_STATISTICS
	}

    void Renderer2D::DrawUnitQuad()
    {
		RenderCommand::DrawTriangleStrip(s_Data.UnitQuadVA, 4);
    }

    void Renderer2D::DrawQuad(const Math::mat4& transform, const Math::vec3& color)
	{
		DrawQuad(transform, { color.r, color.g, color.b, 1.0f });
	}

	void Renderer2D::DrawQuad(const Math::mat4& transform, const Math::vec4& color, int entityID)
	{
		VX_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DInternalData::MaxIndices)
			NextBatch();

		constexpr float textureIndex = 0.0f; // Our White Texture
		constexpr Math::vec2 textureScale = Math::vec2(1.0f);

		Math::vec2 textureCoords[4] = { {0.0f, 0.0f}, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		AddToQuadVertexBuffer(transform, color, textureCoords, textureIndex, textureScale, entityID);
	}

	void Renderer2D::DrawQuad(const Math::mat4& transform, Color color)
	{
		DrawQuad(transform, ColorToVec4(color));
	}

	void Renderer2D::DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor)
	{
		DrawQuad(transform, texture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor, int entityID)
	{
		VX_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DInternalData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;

		// Find the texture ID for the given texture so we can give it to the vertex descriptions
		for (size_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer2DInternalData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		Math::vec2 textureCoords[4] = { {0.0f, 0.0f}, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		// Vertex Descriptions
		AddToQuadVertexBuffer(transform, tintColor, textureCoords, textureIndex, scale, entityID);
	}

	void Renderer2D::DrawQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor)
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

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color, int entityID)
	{
		Math::mat4 transform = Math::Translate(position) * Math::Scale({ size.x, size.y, 1.0f });
		DrawQuad(transform, color, entityID);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, Color color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, Color color)
	{
		DrawQuad(position, size, ColorToVec4(color));
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor)
	{
		Math::mat4 transform = Math::Translate(position) * Math::Scale({ size.x, size.y, 1.0f });
		DrawQuad(transform, texture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor)
	{
		DrawQuad(position, size, texture, scale, ColorToVec4(tintColor));
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec3& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor)
	{
		VX_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DInternalData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;

		const SharedRef<Texture2D>& texture = subtexture->GetTexure();

		// Find the texture ID for the given texture so we can give it to the vertex descriptions
		for (size_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer2DInternalData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		Math::mat4 transform = Math::Translate(position) * Math::Scale({ size.x, size.y, 1.0f });
		const Math::vec2* textureCoords = subtexture->GetTextureCoords();

		// Vertex Descriptions
		AddToQuadVertexBuffer(transform, tintColor, textureCoords, textureIndex, scale);
	}

	void Renderer2D::DrawQuad(const Math::vec2& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawQuad(const Math::vec3& position, const Math::vec2& size, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor)
	{
		DrawQuad(position, size, subtexture, scale, ColorToVec4(tintColor));
	}

	void Renderer2D::DrawAABB(const Math::AABB& aabb, const Math::mat4& transform, const Math::vec4& color)
	{
		Math::vec4 min = { aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f };
		Math::vec4 max = { aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f };

		Math::vec4 corners[8] =
		{
			transform * Math::vec4 { min.x, min.y, max.z, 1.0f },
			transform * Math::vec4 { min.x, max.y, max.z, 1.0f },
			transform * Math::vec4 { max.x, max.y, max.z, 1.0f },
			transform * Math::vec4 { max.x, min.y, max.z, 1.0f },

			transform * Math::vec4 { min.x, min.y, min.z, 1.0f },
			transform * Math::vec4 { min.x, max.y, min.z, 1.0f },
			transform * Math::vec4 { max.x, max.y, min.z, 1.0f },
			transform * Math::vec4 { max.x, min.y, min.z, 1.0f }
		};

		for (uint32_t i = 0; i < 4; i++)
			DrawLine(corners[i], corners[(i + 1) % 4], color);

		for (uint32_t i = 0; i < 4; i++)
			DrawLine(corners[i + 4], corners[((i + 1) % 4) + 4], color);

		for (uint32_t i = 0; i < 4; i++)
			DrawLine(corners[i], corners[i + 4], color);
	}

	void Renderer2D::DrawQuadBillboard(const Math::mat4& cameraView, const Math::vec3& translation, const Math::vec2& size, const Math::vec4& color)
	{
		if (s_Data.QuadIndexCount >= Renderer2DInternalData::MaxIndices)
			NextBatch();

		const float textureIndex = 0.0f; // White Texture
		const Math::vec2 textureScale = Math::vec2(1.0f);

		Math::vec3 camRightWS = { cameraView[0][0], cameraView[1][0], cameraView[2][0] };
		Math::vec3 camUpWS = { cameraView[0][1], cameraView[1][1], cameraView[2][1] };

		s_Data.QuadVertexBufferPtr->Position = translation + camRightWS * (s_Data.QuadVertexPositions[0].x) * size.x + camUpWS * s_Data.QuadVertexPositions[0].y * size.y;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TexScale = textureScale;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = translation + camRightWS * s_Data.QuadVertexPositions[1].x * size.x + camUpWS * s_Data.QuadVertexPositions[1].y * size.y;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TexScale = textureScale;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = translation + camRightWS * s_Data.QuadVertexPositions[2].x * size.x + camUpWS * s_Data.QuadVertexPositions[2].y * size.y;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TexScale = textureScale;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = translation + camRightWS * s_Data.QuadVertexPositions[3].x * size.x + camUpWS * s_Data.QuadVertexPositions[3].y * size.y;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TexScale = textureScale;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;

		s_Data.Renderer2DStatistics.QuadCount++;
	}

	void Renderer2D::DrawQuadBillboard(const Math::mat4& cameraView, const Math::vec3& translation, const SharedRef<Texture2D>& texture, const Math::vec2& size, const Math::vec4& color, int entityID)
	{
		if (s_Data.QuadIndexCount >= Renderer2DInternalData::MaxIndices)
			NextBatch();

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

		const Math::vec2 textureScale = Math::vec2(1.0f);

		Math::vec3 camRightWS = { cameraView[0][0], cameraView[1][0], cameraView[2][0] };
		Math::vec3 camUpWS = { cameraView[0][1], cameraView[1][1], cameraView[2][1] };

		s_Data.QuadVertexBufferPtr->Position = translation + camRightWS * s_Data.QuadVertexPositions[0].x * size.x + camUpWS * s_Data.QuadVertexPositions[0].y * size.y;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TexScale = textureScale;
		s_Data.QuadVertexBufferPtr->EntityID = entityID;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = translation + camRightWS * s_Data.QuadVertexPositions[1].x * size.x + camUpWS * s_Data.QuadVertexPositions[1].y * size.y;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TexScale = textureScale;
		s_Data.QuadVertexBufferPtr->EntityID = entityID;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = translation + camRightWS * s_Data.QuadVertexPositions[2].x * size.x + camUpWS * s_Data.QuadVertexPositions[2].y * size.y;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TexScale = textureScale;
		s_Data.QuadVertexBufferPtr->EntityID = entityID;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = translation + camRightWS * s_Data.QuadVertexPositions[3].x * size.x + camUpWS * s_Data.QuadVertexPositions[3].y * size.y;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TexScale = textureScale;
		s_Data.QuadVertexBufferPtr->EntityID = entityID;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;

		s_Data.Renderer2DStatistics.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const Math::vec3& color)
	{
		DrawRotatedQuad(transform, { color.r, color.g, color.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const Math::vec4& color, int entityID)
	{
		VX_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DInternalData::MaxIndices)
			NextBatch();

		static constexpr float textureIndex = 0.0f; // Our White Texture
		static constexpr Math::vec2 textureScale = Math::vec2(1.0f);

		Math::vec2 textureCoords[4] = { {0.0f, 0.0f}, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		AddToQuadVertexBuffer(transform, color, textureCoords, textureIndex, textureScale, entityID);
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, Color color)
	{
		DrawRotatedQuad(transform, ColorToVec4(color));
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad(transform, texture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor)
	{
		VX_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DInternalData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;

		for (size_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer2DInternalData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		Math::vec2 textureCoords[4] = { {0.0f, 0.0f}, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		AddToQuadVertexBuffer(transform, tintColor, textureCoords, textureIndex, scale);
	}

	void Renderer2D::DrawRotatedQuad(const Math::mat4& transform, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor)
	{
		DrawRotatedQuad(transform, texture, scale, ColorToVec4(tintColor));
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec3& color, int entityID)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, { color.r, color.g, color.b, 1.0f }, entityID);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const Math::vec4& color, int entityID)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color, entityID);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec3& color, int entityID)
	{
		DrawRotatedQuad(position, size, rotation, { color.r, color.g, color.b, 1.0f }, entityID);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const Math::vec4& color, int entityID)
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

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad(position, size, rotation, texture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, const Math::vec4& tintColor)
	{
		Math::mat4 transform = Math::Translate(position) * Math::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Math::Scale({ size.x, size.y, 1.0f });
		DrawRotatedQuad(transform, texture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subtexture, scale, { tintColor.r, tintColor.g, tintColor.b, 1.0f });
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec3& tintColor)
	{
		DrawRotatedQuad(position, size, rotation, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, const Math::vec4& tintColor)
	{
		VX_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DInternalData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;

		const SharedRef<Texture2D>& texture = subtexture->GetTexure();

		for (size_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer2DInternalData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		Math::mat4 transform = Math::Translate(position) * Math::Rotate(rotation, { 0.0f, 0.0f, 1.0f }) * Math::Scale({ size.x, size.y, 1.0f });
		const Math::vec2* textureCoords = subtexture->GetTextureCoords();

		AddToQuadVertexBuffer(transform, tintColor, textureCoords, textureIndex, scale);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<Texture2D>& texture, const Math::vec2& scale, Color tintColor)
	{
		DrawRotatedQuad(position, size, rotation, texture, scale, ColorToVec4(tintColor));
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec2& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subtexture, scale, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const Math::vec3& position, const Math::vec2& size, float rotation, const SharedRef<SubTexture2D>& subtexture, const Math::vec2& scale, Color tintColor)
	{
		DrawRotatedQuad(position, size, rotation, subtexture, scale, ColorToVec4(tintColor));
	}

	void Renderer2D::DrawSprite(const Math::mat4& transform, SpriteRendererComponent& sprite, int entityID)
	{
		if (sprite.Texture)
			DrawQuad(transform, sprite.Texture, sprite.Scale, sprite.SpriteColor, entityID);
		else
			DrawQuad(transform, sprite.SpriteColor, entityID);
	}

	void Renderer2D::DrawCircle(const Math::mat4& transform, const Math::vec4& color, float thickness, float fade, int entityID)
	{
		VX_PROFILE_FUNCTION();

		// TODO: Implement for Circles
		//if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
			//NextBatch();

		AddToCircleVertexBuffer(transform, color, thickness, fade, entityID);
	}

	void Renderer2D::DrawLine(const Math::vec3& start, const Math::vec3& end, const Math::vec4& color, int entityID)
	{
		for (size_t i = 0; i < 2; i++)
		{
			s_Data.LineVertexBufferPtr->Position = i == 0 ? start : end;
			s_Data.LineVertexBufferPtr->Color = color;
			s_Data.LineVertexBufferPtr->EntityID = entityID;
			s_Data.LineVertexBufferPtr++;
		}

		s_Data.LineVertexCount += 2;

#if VX_RENDERER_STATISTICS
		s_Data.Renderer2DStatistics.LineCount++;
#endif // SP_RENDERER_STATISTICS
	}

	void Renderer2D::DrawRect(const Math::mat4& transform, const Math::vec4& color, int entityID)
	{
		Math::vec3 lineVertices[4];

		for (size_t i = 0; i < 4; i++)
			lineVertices[i] = transform * s_Data.QuadVertexPositions[i];

		DrawLine(lineVertices[0], lineVertices[1], color, entityID);
		DrawLine(lineVertices[1], lineVertices[2], color, entityID);
		DrawLine(lineVertices[2], lineVertices[3], color, entityID);
		DrawLine(lineVertices[3], lineVertices[0], color, entityID);
	}

	void Renderer2D::DrawRect(const Math::vec3& position, const Math::vec2& size, const Math::vec4& color, int entityID)
	{
		Math::vec3 bottomLeft = { position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z };
		Math::vec3 bottomRight = { position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z };
		Math::vec3 topRight = { position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z };
		Math::vec3 topLeft = { position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z };

		DrawLine(bottomLeft, bottomRight, color, entityID);
		DrawLine(bottomRight, topRight, color, entityID);
		DrawLine(topRight, topLeft, color, entityID);
		DrawLine(topLeft, bottomLeft, color, entityID);
	}

	void Renderer2D::DrawString(const std::string& string, const Math::vec3& position, float maxWidth, const Math::vec4& color, int entityID)
	{
		DrawString(string, Font::GetDefaultFont(), position, maxWidth, color, entityID);
	}

	void Renderer2D::DrawString(const std::string& string, const SharedRef<Font>& font, const Math::vec3& position, float maxWidth, const Math::vec4& color, int entityID)
	{
		DrawString(string, font, Math::Identity() * Math::Translate(position), maxWidth, color, entityID);
	}

	static bool NextLine(int index, const std::vector<int>& lines)
	{
		for (int line : lines)
		{
			if (line == index)
				return true;
		}

		return false;
	}

	// warning C4996: 'std::codecvt_utf8<char32_t,1114111,(std::codecvt_mode)0>': warning STL4017: std::wbuffer_convert, std::wstring_convert, and the <codecvt> header
// (containing std::codecvt_mode, std::codecvt_utf8, std::codecvt_utf16, and std::codecvt_utf8_utf16) are deprecated in C++17. (The std::codecvt class template is NOT deprecated.)
// The C++ Standard doesn't provide equivalent non-deprecated functionality; consider using MultiByteToWideChar() and WideCharToMultiByte() from <Windows.h> instead.
// You can define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning.
#pragma warning(disable : 4996)

	// From https://stackoverflow.com/questions/31302506/stdu32string-conversion-to-from-stdstring-and-stdu16string
	static std::u32string To_UTF32(const std::string& s)
	{
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
		return conv.from_bytes(s);
	}

#pragma warning(default : 4996)

	void Renderer2D::DrawString(const std::string& string, const SharedRef<Font>& font, const Math::mat4& transform, float maxWidth, const Math::vec4& color, float lineHeightOffset, float kerningOffset, int entityID)
	{
		if (string.empty())
			return;

		float textureIndex = 0.0f;

		// This is not ideal (WIP)
		std::u32string utf32string = To_UTF32(string);

		SharedRef<Texture2D> fontAtlas = font->GetFontAtlas();
		VX_CORE_ASSERT(fontAtlas, "Font Atlas was null pointer!");

		if (!fontAtlas->IsLoaded())
			return;

		for (uint32_t i = 0; i < s_Data.FontTextureSlotIndex; i++)
		{
			if (*s_Data.FontTextureSlots[i].get() == *fontAtlas.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.FontTextureSlotIndex >= Renderer2DInternalData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)s_Data.FontTextureSlotIndex;
			s_Data.FontTextureSlots[s_Data.FontTextureSlotIndex] = fontAtlas;
			s_Data.FontTextureSlotIndex++;
		}

		auto& fontGeometry = font->GetMSDFData()->FontGeometry;
		const auto& metrics = fontGeometry.getMetrics();

		std::vector<int> nextLines;
		{
			double x = 0.0;
			double fsScale = 1 / (metrics.ascenderY - metrics.descenderY);
			double y = -fsScale * metrics.ascenderY;
			int lastSpace = -1;
			for (int i = 0; i < utf32string.size(); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n')
				{
					x = 0;
					y -= fsScale * metrics.lineHeight + lineHeightOffset;
					continue;
				}

				auto glyph = fontGeometry.getGlyph(character);
				if (!glyph)
					glyph = fontGeometry.getGlyph('?');
				if (!glyph)
					continue;

				if (character != ' ')
				{
					// Calc geo
					double pl, pb, pr, pt;
					glyph->getQuadPlaneBounds(pl, pb, pr, pt);
					Math::vec2 quadMin((float)pl, (float)pb);
					Math::vec2 quadMax((float)pr, (float)pt);

					quadMin *= fsScale;
					quadMax *= fsScale;
					quadMin += Math::vec2(x, y);
					quadMax += Math::vec2(x, y);

					if (quadMax.x > maxWidth && lastSpace != -1)
					{
						i = lastSpace;
						nextLines.emplace_back(lastSpace);
						lastSpace = -1;
						x = 0;
						y -= fsScale * metrics.lineHeight + lineHeightOffset;
					}
				}
				else
				{
					lastSpace = i;
				}

				double advance = glyph->getAdvance();
				fontGeometry.getAdvance(advance, character, utf32string[i + 1]);
				x += fsScale * advance + kerningOffset;
			}

			{
				double x = 0.0;
				double fsScale = 1 / (metrics.ascenderY - metrics.descenderY);
				double y = 0.0;// -fsScale * metrics.ascenderY;
				for (int i = 0; i < utf32string.size(); i++)
				{
					char32_t character = utf32string[i];
					if (character == '\n' || NextLine(i, nextLines))
					{
						x = 0;
						y -= fsScale * metrics.lineHeight + lineHeightOffset;
						continue;
					}

					auto glyph = fontGeometry.getGlyph(character);
					if (!glyph)
						glyph = fontGeometry.getGlyph('?');
					if (!glyph)
						continue;

					double l, b, r, t;
					glyph->getQuadAtlasBounds(l, b, r, t);

					double pl, pb, pr, pt;
					glyph->getQuadPlaneBounds(pl, pb, pr, pt);

					pl *= fsScale, pb *= fsScale, pr *= fsScale, pt *= fsScale;
					pl += x, pb += y, pr += x, pt += y;

					double texelWidth = 1.0 / fontAtlas->GetWidth();
					double texelHeight = 1.0 / fontAtlas->GetHeight();
					l *= texelWidth, b *= texelHeight, r *= texelWidth, t *= texelHeight;

					s_Data.TextVertexBufferPtr->Position = transform * Math::vec4(pl, pb, 0.0f, 1.0f);
					s_Data.TextVertexBufferPtr->Color = color;
					s_Data.TextVertexBufferPtr->TexCoord = { l, b };
					s_Data.TextVertexBufferPtr->TexIndex = textureIndex;
					s_Data.TextVertexBufferPtr->EntityID = entityID;
					s_Data.TextVertexBufferPtr++;

					s_Data.TextVertexBufferPtr->Position = transform * Math::vec4(pl, pt, 0.0f, 1.0f);
					s_Data.TextVertexBufferPtr->Color = color;
					s_Data.TextVertexBufferPtr->TexCoord = { l, t };
					s_Data.TextVertexBufferPtr->TexIndex = textureIndex;
					s_Data.TextVertexBufferPtr->EntityID = entityID;
					s_Data.TextVertexBufferPtr++;

					s_Data.TextVertexBufferPtr->Position = transform * Math::vec4(pr, pt, 0.0f, 1.0f);
					s_Data.TextVertexBufferPtr->Color = color;
					s_Data.TextVertexBufferPtr->TexCoord = { r, t };
					s_Data.TextVertexBufferPtr->TexIndex = textureIndex;
					s_Data.TextVertexBufferPtr->EntityID = entityID;
					s_Data.TextVertexBufferPtr++;

					s_Data.TextVertexBufferPtr->Position = transform * Math::vec4(pr, pb, 0.0f, 1.0f);
					s_Data.TextVertexBufferPtr->Color = color;
					s_Data.TextVertexBufferPtr->TexCoord = { r, b };
					s_Data.TextVertexBufferPtr->TexIndex = textureIndex;
					s_Data.TextVertexBufferPtr->EntityID = entityID;
					s_Data.TextVertexBufferPtr++;

					s_Data.TextIndexCount += 6;

					double advance = glyph->getAdvance();
					fontGeometry.getAdvance(advance, character, utf32string[(size_t)i + 1]);
					x += fsScale * advance + kerningOffset;

#if VX_RENDERER_STATISTICS
					s_Data.Renderer2DStatistics.QuadCount++;
#endif // SP_RENDERER_STATISTICS
				}
			}
		}
	}

	float Renderer2D::GetLineWidth()
	{
		return s_Data.LineWidth;
	}

	void Renderer2D::SetLineWidth(float width)
	{
		s_Data.LineWidth = width;
		RenderCommand::SetLineSize(width);
	}

	RendererAPI::TriangleCullMode Renderer2D::GetCullMode()
	{
		return s_Data.CullMode;
	}

	void Renderer2D::SetCullMode(RendererAPI::TriangleCullMode cullMode)
	{
		s_Data.CullMode = cullMode;
		RenderCommand::SetCullMode(cullMode);
	}

	RenderStatistics Renderer2D::GetStats()
	{
		return s_Data.Renderer2DStatistics;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Renderer2DStatistics, NULL, sizeof(s_Data.Renderer2DStatistics));
	}

	SharedRef<ShaderLibrary> Renderer2D::GetShaderLibrary()
	{
		return s_Data.ShaderLibrary;
	}

}
