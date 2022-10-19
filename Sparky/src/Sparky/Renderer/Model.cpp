#include "sppch.h"
#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Sparky {

	namespace Utils {

		static std::vector<ModelVertexInfo> LoadModelFromFile(const std::string& filepath, const Math::mat4& transform)
		{
			tinyobj::attrib_t attributes;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;

			std::string warning;
			std::string error;

			bool success = tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, filepath.c_str());
			SP_CORE_ASSERT(success, "{}: {}", warning, error);

			std::vector<ModelVertexInfo> vertices;

			for (const auto& shape : shapes)
			{
				for (const auto& index : shape.mesh.indices)
				{
					Math::vec4 vertexPosition = {
						attributes.vertices[3 * index.vertex_index],
						attributes.vertices[3 * index.vertex_index + 1],
						attributes.vertices[3 * index.vertex_index + 2],
						1.0f
					};

					vertexPosition = transform * vertexPosition;

					Math::vec3 vertexNormal = {
						attributes.normals[3 * index.normal_index],
						attributes.normals[3 * index.normal_index + 1],
						attributes.normals[3 * index.normal_index + 2]
					};

					vertexNormal = Math::Normalize(Math::mat3(transform) * vertexNormal);

					Math::vec2 vertexTexCoord = {
						attributes.texcoords[2 * index.texcoord_index],
						attributes.texcoords[2 * index.texcoord_index + 1]
					};

					vertices.emplace_back(vertexPosition, vertexNormal, vertexTexCoord);
				}
			}

			return vertices;
		}

	}

	Model::Model(const std::string& filepath, const Math::mat4& transform, const Math::vec4& color, int entityID)
		: m_Filepath(filepath)
	{
		auto vertexInfo = Utils::LoadModelFromFile(m_Filepath, transform);

		m_ModelVertices = std::vector<ModelVertex>(vertexInfo.size());

		uint32_t i = 0;
		for (auto& vertex : m_ModelVertices)
		{
			vertex.Position = vertexInfo[i].Position;
			vertex.Color = color;
			vertex.Normal = vertexInfo[i].Normal;
			vertex.TextureCoord = vertexInfo[i].TextureCoord;
			vertex.EntityID = entityID;

			i++;
		}

		m_Vao = VertexArray::Create();

		m_Vbo = VertexBuffer::Create(m_ModelVertices.size());
		m_Vbo->SetLayout({
			{ ShaderDataType::Float4, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Float3, "a_Normal"   },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Int,    "a_EntityID" }
		});

		m_Vao->AddVertexBuffer(m_Vbo);
	}

	void Model::Init()
	{
		s_ModelShader = Shader::Create("Assets/Shaders/Renderer_Model.glsl");
	}

	void Model::OnUpdate(const EditorCamera& camera, const Math::mat4& transform, const Math::vec4& color)
	{
		s_ModelShader->Enable();
		s_ModelShader->SetMat4("u_ViewProjection", camera.GetViewProjection());

		for (auto& vertex : m_ModelVertices)
		{
			//vertex.Position = transform * vertex.Position;
			vertex.Color = color;
		}

		uint32_t size = (uint32_t)m_ModelVertices.size();
		m_Vbo->SetData(m_ModelVertices.data(), size);

		m_VertexCount = size;
	}

	SharedRef<Model> Model::Create(const std::string& filepath, const Math::mat4& transform, const Math::vec4& color, int entityID)
	{
		return CreateShared<Model>(filepath, transform, color, entityID);
	}

}
