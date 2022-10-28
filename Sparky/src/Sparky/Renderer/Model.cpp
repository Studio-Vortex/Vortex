#include "sppch.h"
#include "Model.h"

#include "Sparky/Scene/Entity.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Sparky {

	namespace Utils {

		static Mesh LoadMeshFromFile(const std::string& filepath, const Math::mat4& transform)
		{
			tinyobj::attrib_t attributes;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string warning;
			std::string error;

			bool success = tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, filepath.c_str());
			SP_CORE_ASSERT(success, "{}: {}", warning, error);

			std::vector<ModelVertexInfo> vertices;
			std::vector<uint32_t> indices;
			std::vector<SharedRef<Texture2D>> textures;

			std::unordered_map<ModelVertexInfo, uint32_t> uniqueVertices{};

			for (const auto& shape : shapes)
			{
				for (const auto& index : shape.mesh.indices)
				{
					Math::vec4 vertexPosition = {
						attributes.vertices[3 * index.vertex_index + 0],
						attributes.vertices[3 * index.vertex_index + 1],
						attributes.vertices[3 * index.vertex_index + 2],
						1.0f
					};

					vertexPosition = transform * vertexPosition;

					Math::vec3 vertexNormal = {
						attributes.normals[3 * index.normal_index + 0],
						attributes.normals[3 * index.normal_index + 1],
						attributes.normals[3 * index.normal_index + 2]
					};

					vertexNormal = Math::Normalize(Math::mat3(transform) * vertexNormal);

					Math::vec2 vertexTexCoord = {
						attributes.texcoords[2 * index.texcoord_index + 0],
						attributes.texcoords[2 * index.texcoord_index + 1]
					};

					ModelVertexInfo vertex = { Math::vec3(vertexPosition), vertexNormal, vertexTexCoord };

					if (uniqueVertices.count(vertex) == 0)
					{
						uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(vertex);
					}

					indices.push_back(uniqueVertices[vertex]);
				}
			}

			return { vertices, indices };
		}

	}

	static constexpr const char* DEFAULT_MESH_SOURCE_PATHS[] = {
		"Resources/Meshes/Default/Cube.obj",
		"Resources/Meshes/Default/Sphere.obj",
		"Resources/Meshes/Default/Capsule.obj",
		"Resources/Meshes/Default/Cone.obj",
		"Resources/Meshes/Default/Cylinder.obj",
		"Resources/Meshes/Default/Plane.obj",
		"Resources/Meshes/Default/Torus.obj",
	};

	Model::Model(const std::string& filepath, Entity entity, const Math::vec4& color)
		: m_Filepath(filepath)
	{
		Mesh mesh = Utils::LoadMeshFromFile(m_Filepath, entity.GetTransform().GetTransform());

		m_Vertices = std::vector<ModelVertex>(mesh.Vertices.size());

		uint32_t i = 0;
		for (const auto& vertex : mesh.Vertices)
		{
			ModelVertex& v = m_Vertices[i++];
			v.Position = vertex.Position;
			v.Color = color;
			v.Normal = vertex.Normal;
			v.TextureCoord = vertex.TextureCoord;
			v.TexScale = 1.0f;
			v.EntityID = (int)(entt::entity)entity;
		}

		m_Vao = VertexArray::Create();

		uint32_t indexCount = mesh.Indices.size();
		
		uint32_t dataSize = m_Vertices.size() * sizeof(ModelVertex);
		m_Vbo = VertexBuffer::Create(m_Vertices.data(), dataSize);
		m_Vbo->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal"   },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float,  "a_TexScale" },
			{ ShaderDataType::Int,    "a_EntityID" },
		});

		m_Vao->AddVertexBuffer(m_Vbo);

		m_Ibo = IndexBuffer::Create(mesh.Indices.data(), indexCount);
		m_Vao->SetIndexBuffer(m_Ibo);
	}

	Model::Model(MeshRendererComponent::MeshType meshType)
	{
		Mesh mesh = Utils::LoadMeshFromFile(DEFAULT_MESH_SOURCE_PATHS[static_cast<uint32_t>(meshType)], Math::Identity());

		m_Vao = VertexArray::Create();

		float vertices[] =
		{
			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,

			-0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,

			-0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,

			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,

			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f, -0.5f,

			-0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f, -0.5f,
		};

		uint32_t dataSize = 36 * sizeof(Math::vec3);
		m_Vbo = VertexBuffer::Create(vertices, dataSize);
		m_Vbo->SetLayout({ { ShaderDataType::Float3, "a_Position" } });
		m_Vao->AddVertexBuffer(m_Vbo);
	}

	void Model::OnUpdate(const TransformComponent& transform, const Math::vec4& color, float scale)
	{
		Math::mat4 entityTransform = transform.GetTransform();

		bool recalculateTransform = m_Transform != entityTransform;

		for (auto& vertex : m_Vertices)
		{
			if (recalculateTransform)
			{
				vertex.Position = entityTransform * Math::vec4(vertex.Position, 1.0f);
				m_Transform = entityTransform;
			}

			vertex.Color = color;
			vertex.TexScale = scale;
		}

		uint32_t dataSize = m_Vertices.size() * sizeof(ModelVertex);
		m_Vbo->SetData(m_Vertices.data(), dataSize);
	}

	uint32_t Model::GetQuadCount() const
	{
		return m_Ibo->GetCount() / 3;
	}

	SharedRef<Model> Model::Create(const std::string& filepath, Entity entity, const Math::vec4& color)
	{
		return CreateShared<Model>(filepath, entity, color);
	}

	SharedRef<Model> Model::Create(MeshRendererComponent::MeshType meshType)
	{
		return CreateShared<Model>(meshType);
	}

}
