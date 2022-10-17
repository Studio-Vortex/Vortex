#include "sppch.h"
#include "Renderer.h"

#include "Sparky/Core/Base.h"
#include "Sparky/Renderer/Renderer2D.h"

namespace Sparky {

	static constexpr const char* CUBE_SHADER_PATH = "Assets/Shaders/Renderer_Cube.glsl";

	struct CubeVertex
	{
		Math::vec3 Position;
		Math::vec3 Normal;
		Math::vec4 Color;
		Math::vec2 TexCoord;
		float TexIndex;
		float TexScale;

		// Editor-only
		int EntityID;
	};

	struct RendererInternalData
	{
		static constexpr inline uint32_t MaxCubes = 20'000;
		static constexpr inline uint32_t MaxVertices = MaxCubes * VERTICES_PER_CUBE;
		static constexpr inline uint32_t MaxIndices = MaxCubes * INDICES_PER_CUBE;
		static constexpr inline uint32_t MaxTextureSlots = 32; // TODO: RendererCapabilities

		SharedRef<Texture2D> WhiteTexture; // Default texture

		SharedRef<VertexArray> CubeVA;
		SharedRef<VertexBuffer> CubeVB;
		SharedRef<Shader> CubeShader;

		uint32_t CubeIndexCount = 0;
		CubeVertex* CubeVertexBufferBase = nullptr;
		CubeVertex* CubeVertexBufferPtr = nullptr;

		std::array<SharedRef<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture

		Math::vec4 CubeVertexPositions[24];

		RenderStatistics RendererStatistics;
		RendererAPI::TriangleCullMode CullMode = RendererAPI::TriangleCullMode::None;
	};

	static RendererInternalData s_Data;

	void Renderer::Init()
	{
		SP_PROFILE_FUNCTION();

		RenderCommand::Init();

		s_Data.CubeVA = VertexArray::Create();

		s_Data.CubeVB = VertexBuffer::Create(RendererInternalData::MaxVertices * sizeof(CubeVertex));
		s_Data.CubeVB->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal"   },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float,  "a_TexIndex" },
			{ ShaderDataType::Float,  "a_TexScale" },
			{ ShaderDataType::Int,    "a_EntityID" },
		});

		s_Data.CubeVA->AddVertexBuffer(s_Data.CubeVB);

		s_Data.CubeVertexBufferBase = new CubeVertex[RendererInternalData::MaxVertices];

		uint32_t* cubeIndices = new uint32_t[RendererInternalData::MaxIndices];

		uint32_t offset = 0;
		for (size_t i = 0; i < RendererInternalData::MaxIndices; i += INDICES_PER_CUBE)
		{
			cubeIndices[i + 0] = offset + 0;
			cubeIndices[i + 1] = offset + 1;
			cubeIndices[i + 2] = offset + 2;

			cubeIndices[i + 3] = offset + 2;
			cubeIndices[i + 4] = offset + 3;
			cubeIndices[i + 5] = offset + 0;

			cubeIndices[i + 6] = offset + 4;
			cubeIndices[i + 7] = offset + 5;
			cubeIndices[i + 8] = offset + 7;

			cubeIndices[i + 9] = offset + 7;
			cubeIndices[i + 10] = offset + 5;
			cubeIndices[i + 11] = offset + 6;

			cubeIndices[i + 12] = offset + 8;
			cubeIndices[i + 13] = offset + 9;
			cubeIndices[i + 14] = offset + 11;

			cubeIndices[i + 15] = offset + 11;
			cubeIndices[i + 16] = offset + 9;
			cubeIndices[i + 17] = offset + 10;

			cubeIndices[i + 18] = offset + 12;
			cubeIndices[i + 19] = offset + 13;
			cubeIndices[i + 20] = offset + 15;

			cubeIndices[i + 21] = offset + 15;
			cubeIndices[i + 22] = offset + 13;
			cubeIndices[i + 23] = offset + 14;

			cubeIndices[i + 24] = offset + 16;
			cubeIndices[i + 25] = offset + 17;
			cubeIndices[i + 26] = offset + 19;

			cubeIndices[i + 27] = offset + 19;
			cubeIndices[i + 28] = offset + 17;
			cubeIndices[i + 29] = offset + 18;

			cubeIndices[i + 30] = offset + 20;
			cubeIndices[i + 31] = offset + 21;
			cubeIndices[i + 32] = offset + 23;

			cubeIndices[i + 33] = offset + 23;
			cubeIndices[i + 34] = offset + 21;
			cubeIndices[i + 35] = offset + 22;

			offset += VERTICES_PER_CUBE;
		}

		SharedRef<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, RendererInternalData::MaxIndices);
		s_Data.CubeVA->SetIndexBuffer(cubeIB);
		delete[] cubeIndices;

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		// Gpu Sampler2D Array
		int32_t samplers[s_Data.MaxTextureSlots];
		for (size_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		// Set the first texture slot to out default white texture
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		s_Data.CubeShader = Shader::Create(CUBE_SHADER_PATH);
		s_Data.CubeShader->Enable();

		s_Data.CubeShader->SetIntArray("u_Textures", samplers, RendererInternalData::MaxTextureSlots);

		// Create a cube's set of vertices at the origin in ndc
		s_Data.CubeVertexPositions[ 0] = { -0.5f,  0.5f,  0.5f, 1.0f };
		s_Data.CubeVertexPositions[ 1] = {  0.5f,  0.5f,  0.5f, 1.0f };
		s_Data.CubeVertexPositions[ 2] = {  0.5f, -0.5f,  0.5f, 1.0f };
		s_Data.CubeVertexPositions[ 3] = { -0.5f, -0.5f,  0.5f, 1.0f }; // Front face

		s_Data.CubeVertexPositions[ 4] = {  0.5f,  0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[ 5] = { -0.5f,  0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[ 6] = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[ 7] = {  0.5f, -0.5f, -0.5f, 1.0f }; // Back face

		s_Data.CubeVertexPositions[ 8] = { -0.5f,  0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[ 9] = { -0.5f,  0.5f,  0.5f, 1.0f };
		s_Data.CubeVertexPositions[10] = { -0.5f, -0.5f,  0.5f, 1.0f };
		s_Data.CubeVertexPositions[11] = { -0.5f, -0.5f, -0.5f, 1.0f }; // Left face

		s_Data.CubeVertexPositions[12] = {  0.5f,  0.5f,  0.5f, 1.0f };
		s_Data.CubeVertexPositions[13] = {  0.5f,  0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[14] = {  0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[15] = {  0.5f, -0.5f,  0.5f, 1.0f }; // Right face

		s_Data.CubeVertexPositions[16] = { -0.5f,  0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[17] = {  0.5f,  0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[18] = {  0.5f,  0.5f,  0.5f, 1.0f };
		s_Data.CubeVertexPositions[19] = { -0.5f,  0.5f,  0.5f, 1.0f }; // Top face

		s_Data.CubeVertexPositions[20] = { -0.5f, -0.5f,  0.5f, 1.0f };
		s_Data.CubeVertexPositions[21] = {  0.5f, -0.5f,  0.5f, 1.0f };
		s_Data.CubeVertexPositions[22] = {  0.5f, -0.5f, -0.5f, 1.0f };
		s_Data.CubeVertexPositions[23] = { -0.5f, -0.5f, -0.5f, 1.0f }; // Bottom face

#if SP_RENDERER_STATISTICS
		ResetStats();
#endif // SP_RENDERER_STATISTICS

		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		SP_PROFILE_FUNCTION();

		delete[] s_Data.CubeVertexBufferBase;

		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(const Viewport& viewport)
	{
		RenderCommand::SetViewport(viewport);
	}

	void Renderer::BeginScene(const Camera& camera, const Math::mat4& transform)
	{
		SP_PROFILE_FUNCTION();

		Math::mat4 viewProjection = camera.GetProjection() * Math::Inverse(transform);

		s_Data.CubeShader->Enable();
		s_Data.CubeShader->SetMat4("u_ViewProjection", viewProjection);

		StartBatch();
	}

	void Renderer::BeginScene(const EditorCamera& camera)
	{
		SP_PROFILE_FUNCTION();

		s_Data.CubeShader->Enable();
		s_Data.CubeShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

		StartBatch();
	}

	void Renderer::StartBatch()
	{
		// Set the index count to 0 for the new batch
		s_Data.CubeIndexCount = 0;
		// Set the pointer to the beginning of the vertex buffer
		s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;

		// Reset the texture slot index (0 is the default white texture)
		s_Data.TextureSlotIndex = 1;
	}

	void Renderer::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer::EndScene()
	{
		SP_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer::Flush()
	{
		/// Cubes
		if (s_Data.CubeIndexCount)
		{
			// Calculate the size of the vertex buffer
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CubeVertexBufferPtr - (uint8_t*)s_Data.CubeVertexBufferBase);
			// Copy data to GPU buffer
			s_Data.CubeVB->SetData(s_Data.CubeVertexBufferBase, dataSize);

			// Bind all textures used in the batch
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);

			// Bind a shader and make a draw call
			s_Data.CubeShader->Enable();
			s_Data.CubeVA->Bind();
			RenderCommand::DrawIndexed(s_Data.CubeVA, s_Data.CubeIndexCount);
			s_Data.RendererStatistics.DrawCalls++;
		}
	}

	void Renderer::AddToCubeVertexBuffer(const Math::mat4& transform, const Math::vec3* normals, const Math::vec4& color, const Math::vec2* textureCoords, float textureIndex, float textureScale, int entityID)
	{
		for (size_t i = 0; i < 24; i++)
		{
			s_Data.CubeVertexBufferPtr->Position = transform * s_Data.CubeVertexPositions[i];
			s_Data.CubeVertexBufferPtr->Normal = normals[i];
			s_Data.CubeVertexBufferPtr->Color = color;
			s_Data.CubeVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CubeVertexBufferPtr->TexScale = textureScale;
			s_Data.CubeVertexBufferPtr->EntityID = entityID;
			s_Data.CubeVertexBufferPtr++;
		}

		s_Data.CubeIndexCount += INDICES_PER_CUBE;

#if SP_RENDERER_STATISTICS
		s_Data.RendererStatistics.QuadCount += 6;
#endif // SP_RENDERER_STATISTICS
	}

	void Renderer::Submit(const SharedRef<Shader>& shader, const SharedRef<VertexArray>& vertexArray, const Math::mat4& transform)
	{
		SP_PROFILE_FUNCTION();

		shader->Enable();
		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

	void Renderer::DrawCube(const Math::mat4& transform, const MeshRendererComponent& meshRenderer, int entityID)
	{
		SP_PROFILE_FUNCTION();

		if (s_Data.CubeIndexCount >= RendererInternalData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f; // Our White Texture

		SharedRef<Texture2D> texture = meshRenderer.Texture;

		if (texture)
		{
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
				if (s_Data.TextureSlotIndex >= RendererInternalData::MaxTextureSlots)
					NextBatch();

				textureIndex = (float)s_Data.TextureSlotIndex;
				s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
				s_Data.TextureSlotIndex++;
			}
		}

		Math::vec2 textureCoords[24] = {
			{ 0.0f, 1.0f }, { 1.0f, 1.0f },
			{ 1.0f, 0.0f }, { 0.0f, 0.0f },      // Front face

			{ 0.0f, 1.0f }, { 1.0f, 1.0f },
			{ 1.0f, 0.0f }, { 0.0f, 0.0f },      // Back face

			{ 0.0f, 1.0f }, { 1.0f, 1.0f },
			{ 1.0f, 0.0f }, { 0.0f, 0.0f },      // Left face

			{ 0.0f, 1.0f }, { 1.0f, 1.0f },
			{ 1.0f, 0.0f }, { 0.0f, 0.0f },      // Right face

			{ 0.0f, 1.0f }, { 1.0f, 1.0f },
			{ 1.0f, 0.0f }, { 0.0f, 0.0f },      // Top face

			{ 0.0f, 1.0f }, { 1.0f, 1.0f },
			{ 1.0f, 0.0f }, { 0.0f, 0.0f },      // Bottom face
		};

		Math::vec3 normals[24] = {
			{ 0.0f,  0.0f,  1.0f },  { 0.0f,  0.0f,  1.0f },
			{ 0.0f,  0.0f,  1.0f },  { 0.0f,  0.0f,  1.0f },          // Front face
	
			{ 0.0f,  0.0f, -1.0f },  { 0.0f,  0.0f, -1.0f },
			{ 0.0f,  0.0f, -1.0f },  { 0.0f,  0.0f, -1.0f },          // Back face
	
			{ -1.0f,  0.0f,  0.0f }, { -1.0f,  0.0f,  0.0f },
			{ -1.0f,  0.0f,  0.0f }, { -1.0f,  0.0f,  0.0f },         // Left face
	
			{ 1.0f,  0.0f,  0.0f },  { 1.0f,  0.0f,  0.0f },
			{ 1.0f,  0.0f,  0.0f },  { 1.0f,  0.0f,  0.0f },          // Right face
	
			{ 0.0f,  1.0f,  0.0f },  { 0.0f,  1.0f,  0.0f },
			{ 0.0f,  1.0f,  0.0f },  { 0.0f,  1.0f,  0.0f },          // Top face
	
			{ 0.0f, -1.0f,  0.0f },  { 0.0f, -1.0f,  0.0f },
			{ 0.0f, -1.0f,  0.0f },  { 0.0f, -1.0f,  0.0f },          // Bottom face
		};

		AddToCubeVertexBuffer(transform, normals, meshRenderer.Color, textureCoords, textureIndex, meshRenderer.Scale, entityID);
	}

	void Renderer::DrawCubeWireframe(const TransformComponent& transform)
	{
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x + (transform.Scale.x / 2.0f), transform.Translation.y, transform.Translation.z }) * Math::Rotate(Math::Deg2Rad(90.0f), { 0.0f, 1.0f, 0.0f }) * Math::Scale({ transform.Scale.z, transform.Scale.y, transform.Scale.z, }), ColorToVec4(Color::Orange));
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x - (transform.Scale.x / 2.0f), transform.Translation.y, transform.Translation.z }) * Math::Rotate(Math::Deg2Rad(90.0f), { 0.0f, 1.0f, 0.0f }) * Math::Scale({ transform.Scale.z, transform.Scale.y, transform.Scale.z, }), ColorToVec4(Color::Orange));

		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y + (transform.Scale.y / 2.0f), transform.Translation.z }) * Math::Rotate(Math::Deg2Rad(90.0f), { 1.0f, 0.0f, 0.0f }) * Math::Scale({ transform.Scale.x, transform.Scale.z, transform.Scale.z, }), ColorToVec4(Color::Orange));
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y - (transform.Scale.y / 2.0f), transform.Translation.z }) * Math::Rotate(Math::Deg2Rad(90.0f), { 1.0f, 0.0f, 0.0f }) * Math::Scale({ transform.Scale.x, transform.Scale.z, transform.Scale.z, }), ColorToVec4(Color::Orange));

		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y, transform.Translation.z + (transform.Scale.z / 2.0f) }) * Math::Scale({ transform.Scale.x, transform.Scale.y, transform.Scale.z, }), ColorToVec4(Color::Orange));
		Renderer2D::DrawRect(transform.GetTransform() * Math::Translate({ transform.Translation.x, transform.Translation.y, transform.Translation.z - (transform.Scale.z / 2.0f) }) * Math::Scale({ transform.Scale.x, transform.Scale.y, transform.Scale.z, }), ColorToVec4(Color::Orange));

	}

	RendererAPI::TriangleCullMode Renderer::GetCullMode()
	{
		return s_Data.CullMode;
	}

	void Renderer::SetCullMode(RendererAPI::TriangleCullMode cullMode)
	{
		s_Data.CullMode = cullMode;
	}

	RenderStatistics Renderer::GetStats()
	{
		return s_Data.RendererStatistics;
	}

	void Renderer::ResetStats()
	{
		memset(&s_Data.RendererStatistics, 0, sizeof(s_Data.RendererStatistics));
	}

}
