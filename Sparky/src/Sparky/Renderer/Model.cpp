#include "sppch.h"
#include "Model.h"

#include "Sparky/Scene/Entity.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <tiny_gltf.h>

namespace Sparky {

	namespace Utils {

		static void GenerateGeometryTangents(std::vector<ModelVertexInfo>& vertices, const std::vector<uint32_t>& indices)
		{
			uint32_t indexCount = indices.size();
			for (uint32_t i = 0; i < indexCount; i += 3)
			{
				uint32_t i0 = indices[(size_t)i + 0];
				uint32_t i1 = indices[(size_t)i + 1];
				uint32_t i2 = indices[(size_t)i + 2];

				Math::vec3 edge1 = vertices[i1].Position - vertices[i0].Position;
				Math::vec3 edge2 = vertices[i2].Position - vertices[i0].Position;

				float deltaU1 = vertices[i1].TextureCoord.x - vertices[i0].TextureCoord.x;
				float deltaV1 = vertices[i1].TextureCoord.y - vertices[i0].TextureCoord.y;

				float deltaU2 = vertices[i2].TextureCoord.x - vertices[i0].TextureCoord.x;
				float deltaV2 = vertices[i2].TextureCoord.y - vertices[i0].TextureCoord.y;

				float dividend = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
				float fc = 1.0f / dividend;

				Math::vec3 tangent(
					(fc * (deltaV2 * edge1.x - deltaV1 * edge2.x)),
					(fc * (deltaV2 * edge1.y - deltaV1 * edge2.y)),
					(fc * (deltaV2 * edge1.z - deltaV1 * edge2.z))
				);

				tangent = Math::Normalize(tangent);
				
				float sx = deltaU1, sy = deltaU2;
				float tx = deltaV1, ty = deltaV2;
				float handedness = ((tx * sy - ty * sx) < 0.0f) ? 1.0f : -1.0f;
				Math::vec4 t4(tangent, handedness);
				vertices[i0].Tangent = t4;
				vertices[i1].Tangent = t4;
				vertices[i2].Tangent = t4;
			}
		}

		static Mesh LoadMeshFromOBJFile(const std::string& filepath, const Math::mat4& transform)
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

					ModelVertexInfo vertex = { Math::vec3(vertexPosition), vertexNormal, Math::vec4(), vertexTexCoord};

					if (uniqueVertices.count(vertex) == 0)
					{
						uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(vertex);
					}

					indices.push_back(uniqueVertices[vertex]);
				}
			}

			GenerateGeometryTangents(vertices, indices);

			return { vertices, indices };
		}

		static Mesh LoadMeshFromGLTFFile(const std::string& filepath, const Math::mat4& transform)
		{
			tinygltf::Model model;
			tinygltf::TinyGLTF loader;
			std::string warning;
			std::string error;

			bool success = loader.LoadASCIIFromFile(&model, &error, &warning, filepath);
			SP_CORE_ASSERT(success, "{}: {}", warning, error);

			return Mesh();
		}

	}

	Model::Model(const std::string& filepath, const TransformComponent& transform, int entityID)
		: m_Filepath(filepath), m_MaterialInstance(MaterialInstance::Create())
	{
		LoadModelFromFile(filepath, transform, entityID);
	}

	Model::Model(const std::string& filepath, const SharedRef<MaterialInstance>& materialInstance, const TransformComponent& transform, int entityID)
		: m_Filepath(filepath), m_MaterialInstance(materialInstance)
	{
		LoadModelFromFile(filepath, transform, entityID);
	}

	Model::Model(Model::Default defaultMesh, const TransformComponent& transform, int entityID)
		: m_MaterialInstance(MaterialInstance::Create())
	{
		m_Filepath = DefaultMeshSourcePaths[static_cast<uint32_t>(defaultMesh)];
		LoadModelFromFile(m_Filepath, transform, entityID);
	}

	Model::Model(MeshType meshType)
		: m_MaterialInstance(nullptr)
	{
		Mesh mesh = Utils::LoadMeshFromOBJFile(DefaultMeshSourcePaths[static_cast<uint32_t>(meshType)], Math::Identity());

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

	void Model::LoadModelFromFile(const std::string& filepath, const TransformComponent& transform, int entityID)
	{
		std::filesystem::path path = std::filesystem::path(filepath);
		Mesh mesh;

		if (path.filename().extension() == ".obj")
			mesh = Utils::LoadMeshFromOBJFile(m_Filepath, transform.GetTransform());
		else if (path.filename().extension() == ".gltf")
			mesh = Utils::LoadMeshFromGLTFFile(m_Filepath, transform.GetTransform());

		m_OriginalVertices.resize(mesh.Vertices.size());

		// Store the original mesh vertices
		uint32_t i = 0;
		for (const auto& vertex : mesh.Vertices)
		{
			ModelVertex& v = m_OriginalVertices[i++];
			v.Position = vertex.Position;
			v.Normal = vertex.Normal;
			v.Tangent = vertex.Tangent;
			v.TextureCoord = vertex.TextureCoord;
			v.TexScale = Math::vec2(1.0f);
			v.EntityID = (int)(entt::entity)entityID;
		}

		m_Vao = VertexArray::Create();

		uint32_t indexCount = mesh.Indices.size();

		uint32_t dataSize = m_OriginalVertices.size() * sizeof(ModelVertex);
		m_Vbo = VertexBuffer::Create(m_OriginalVertices.data(), dataSize);
		m_Vbo->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal"   },
			{ ShaderDataType::Float4, "a_Tangent"  },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float2, "a_TexScale" },
			{ ShaderDataType::Int,    "a_EntityID" },
		});

		m_Vao->AddVertexBuffer(m_Vbo);

		m_Ibo = IndexBuffer::Create(mesh.Indices.data(), indexCount);
		m_Vao->SetIndexBuffer(m_Ibo);

		m_Vertices = m_OriginalVertices;
	}

	void Model::OnUpdate(int entityID, const Math::vec2& scale)
	{
		bool entityIDChanged = m_EntityID != entityID;
		bool scaleChanged = m_TextureScale != scale;

		if (scaleChanged || entityIDChanged)
		{
			for (auto& vertex : m_Vertices)
			{
				vertex.TexScale = scale;
				vertex.EntityID = entityID;
			}

			m_EntityID = entityID;
			m_TextureScale = scale;

			uint32_t dataSize = m_Vertices.size() * sizeof(ModelVertex);
			m_Vbo->SetData(m_Vertices.data(), dataSize);
		}
	}

	uint32_t Model::GetQuadCount() const
	{
		return m_Ibo->GetCount() / 3;
	}

	SharedRef<Model> Model::Create(const std::string& filepath, const TransformComponent& transform, int entityID)
	{
		return CreateShared<Model>(filepath, transform, entityID);
	}

	SharedRef<Model> Model::Create(const std::string& filepath, const SharedRef<MaterialInstance>& materialInstance, const TransformComponent& transform, int entityID)
	{
		return CreateShared<Model>(filepath, transform, entityID);
	}

	SharedRef<Model> Model::Create(Model::Default defaultMesh, const TransformComponent& transform, int entityID)
	{
		return CreateShared<Model>(defaultMesh, transform, entityID);
	}

	SharedRef<Model> Model::Create(MeshType meshType)
	{
		return CreateShared<Model>(meshType);
	}

}
