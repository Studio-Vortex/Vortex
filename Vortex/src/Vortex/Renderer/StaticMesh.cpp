#include "vxpch.h"
#include "StaticMesh.h"

#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Project/Project.h"
#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/AssimpAPIHelpers.h"
#include "Vortex/Utils/FileSystem.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Vortex {

	static const uint32_t s_MeshImportFlags = 
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_JoinIdenticalVertices |
		//aiProcess_GlobalScale |             // e.g. convert cm to m for fbx import (and other formats where cm is native)
		aiProcess_ValidateDataStructure;    // Validation

	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		virtual void write(const char* message) override
		{
			VX_CORE_WARN_TAG("Mesh", "Assimp error: {0}", message);
		}
	};

	StaticSubmesh::StaticSubmesh(const std::string& name, const std::vector<StaticVertex>& vertices, const std::vector<uint32_t>& indices, const SharedRef<Material>& material)
		: m_MeshName(name), m_Vertices(vertices), m_Indices(indices), m_Material(material)
	{
		CreateAndUploadMesh();
	}

	StaticSubmesh::StaticSubmesh(bool skybox)
	{
		m_VertexArray = VertexArray::Create();

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
		m_VertexBuffer = VertexBuffer::Create(vertices, dataSize);
		m_VertexBuffer->SetLayout({ { ShaderDataType::Float3, "a_Position" } });

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);
	}

	void StaticSubmesh::SetMaterial(const SharedRef<Material>& material)
	{
		m_Material = material;
	}

	void StaticSubmesh::CreateAndUploadMesh()
	{
		m_VertexArray = VertexArray::Create();

		uint32_t dataSize = m_Vertices.size() * sizeof(StaticVertex);
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), dataSize);

		m_VertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"    },
			{ ShaderDataType::Float4, "a_Color"       },
			{ ShaderDataType::Float3, "a_Normal"      },
			{ ShaderDataType::Float3, "a_Tangent"     },
			{ ShaderDataType::Float3, "a_BiTangent"   },
			{ ShaderDataType::Float2, "a_TexCoord"    },
			{ ShaderDataType::Float2, "a_TexScale"    },
			{ ShaderDataType::Int,    "a_EntityID"    },
		});

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		CreateBoundingBoxFromVertices();
	}

	void StaticSubmesh::CreateBoundingBoxFromVertices()
	{
		// initialize bounding box to default
		m_BoundingBox.Min = m_Vertices.at(0).Position;
		m_BoundingBox.Max = m_Vertices.at(0).Position;

		for (auto& vertex : m_Vertices)
		{
			for (uint32_t i = 0; i < 3; i++)
			{
				if (vertex.Position[i] < m_BoundingBox.Min[i])
					m_BoundingBox.Min[i] = vertex.Position[i];

				if (vertex.Position[i] > m_BoundingBox.Max[i])
					m_BoundingBox.Max[i] = vertex.Position[i];
			}
		}
	}

	void StaticSubmesh::Render() const
	{
		SharedRef<Shader> shader = m_Material->GetShader();
		m_Material->Bind();

		Renderer::DrawIndexed(shader, m_VertexArray);

		uint32_t triangleCount = m_IndexBuffer->GetCount() / 3;
		Renderer::AddToQuadCountStats(triangleCount / 2);
	}

	void StaticSubmesh::RenderToSkylightShadowMap()
	{
		SharedRef<Shader> shader = Renderer::GetShaderLibrary()->Get("SkyLightShadowMap");

		Renderer::DrawIndexed(shader, m_VertexArray);
	}

	StaticMesh::StaticMesh(StaticMesh::Default defaultMesh, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID)
		: m_ImportOptions(importOptions)
	{
		LogStream::Initialize();

		m_Filepath = DefaultMeshSourcePaths[static_cast<uint32_t>(defaultMesh)];

		VX_CORE_INFO_TAG("Mesh", "Loading Mesh: {}", m_Filepath.c_str());

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(m_Filepath, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes())
		{
			VX_CORE_ERROR_TAG("Mesh", "Failed to load Mesh from: {}", m_Filepath.c_str());
			return;
		}

		m_Scene = scene;

		ProcessNode(m_Scene->mRootNode, m_Scene, importOptions, entityID);

		CreateBoundingBoxFromSubmeshes();
	}

	StaticMesh::StaticMesh(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID)
		: m_ImportOptions(importOptions), m_Filepath(filepath)
	{
		LogStream::Initialize();

		VX_CORE_INFO_TAG("Mesh", "Loading Mesh: {}", m_Filepath.c_str());

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(m_Filepath, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes())
		{
			VX_CORE_ERROR_TAG("Mesh", "Failed to load Mesh from: {}", m_Filepath.c_str());
			return;
		}

		m_Scene = scene;

		ProcessNode(m_Scene->mRootNode, m_Scene, importOptions, entityID);

		CreateBoundingBoxFromSubmeshes();
	}

	StaticMesh::StaticMesh(const std::vector<StaticVertex>& vertices, const std::vector<Index>& indices, const Math::mat4& transform)
	{
		std::vector<StaticVertex> verts = { StaticVertex{} };
		std::vector<uint32_t> inds = { 0 };
		SharedRef<Material> mat = nullptr;

		auto TransformVerticesAndGetIndicesAndCreateMesh =
		[
			&collectionOfMeshes = m_Submeshes,
			transform,
			vertices,
			indices,
			&verts,
			&inds,
			mat
		]()
		{
			auto TransformVertices = [&verts, transform, vertices]()
			{
				auto TransformVertexFunc = [&verts, transform](auto vertex)
				{
					StaticVertex transformedVertex = vertex;
					Math::vec4 transformedPositionAttribute = Math::vec4(transformedVertex.Position, 1.0) * transform;
					transformedVertex.Position = Math::vec3(transformedPositionAttribute);
					verts.push_back(transformedVertex);
				};

				for (const auto& vertex : vertices)
					TransformVertexFunc(vertex);
			};

			auto GetIndices = [&inds, indices]()
			{
				auto GetIndexFunc = [&inds](auto index)
				{
					uint32_t theIndices[3] = { index.i0, index.i1, index.i2 };
					for (uint32_t i = 0; i < VX_ARRAYCOUNT(theIndices); i++)
						inds.push_back(theIndices[i]);
				};

				for (const auto& index : indices)
					GetIndexFunc(index);
			};

			TransformVertices();
			GetIndices();

			StaticSubmesh mesh("UnNamed", verts, inds, mat);
			collectionOfMeshes.push_back(mesh);
		};

		if (vertices.size() > 1 && indices.size() > 1)
		{
			TransformVerticesAndGetIndicesAndCreateMesh();
		}

		CreateBoundingBoxFromSubmeshes();
	}

	StaticMesh::StaticMesh(MeshType meshType)
	{
		m_Submeshes.push_back(StaticSubmesh(true));
	}

	void StaticMesh::ProcessNode(aiNode* node, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID)
	{
		// process all node meshes
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Submeshes.push_back(ProcessMesh(mesh, scene, importOptions, entityID));
		}

		// do the same for children nodes
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, importOptions, entityID);
		}
	}

	StaticSubmesh StaticMesh::ProcessMesh(aiMesh* mesh, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID)
	{
		std::vector<StaticVertex> vertices;
		std::vector<uint32_t> indices;
		SharedRef<Material> material = nullptr;

		const TransformComponent& importTransform = importOptions.MeshTransformation;
		Math::vec3 rotation = importTransform.GetRotationEuler();
		Math::mat4 transform = Math::Translate(importTransform.Translation) *
			Math::Rotate(Math::Deg2Rad(rotation.x), { 1.0f, 0.0f, 0.0f }) *
			Math::Rotate(Math::Deg2Rad(rotation.y), { 0.0f, 1.0f, 0.0f }) *
			Math::Rotate(Math::Deg2Rad(rotation.z), { 0.0f, 0.0f, 1.0f }) *
			Math::Scale(importTransform.Scale);

		const char* nameCStr = mesh->mName.C_Str();
		std::string meshName = std::string(nameCStr);

		// process vertices
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			StaticVertex vertex;

			VX_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions!");
			VX_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals!");

			vertex.Position = Math::vec3(transform * Math::vec4(Math::vec3{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z }, 1.0f)) * 0.5f;

			for (uint32_t j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; j++)
			{
				if (mesh->HasVertexColors(j))
				{
					vertex.Color = { mesh->mColors[j][0].r, mesh->mColors[j][1].g, mesh->mColors[j][2].b, mesh->mColors[j][3].a };
				}
				else
				{
					vertex.Color = Math::vec4(1.0f);
				}
			}

			if (mesh->HasNormals())
			{
				Math::vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
				Math::vec4 normalXYZW(normal, 1.0f);
 				vertex.Normal = Math::vec3(transform * normalXYZW);
			}

			if (mesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = Math::vec3(transform * Math::vec4(Math::vec3{ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z}, 1.0f));
				vertex.BiTangent = Math::vec3(transform * Math::vec4(Math::vec3{ mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z }, 1.0f));
			}

			vertex.TexScale = Math::vec2(1.0f);

			vertex.TexCoord = Math::vec2(0.0f);
			// does it contain texture coords?
			if (mesh->mTextureCoords[0])
			{
				vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			}

			vertex.EntityID = entityID;
			vertices.push_back(vertex);
		}

		// process indices
		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// process materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

			MaterialProperties materialProps;
			materialProps.Name = std::string(mat->GetName().C_Str());

			std::filesystem::path directoryPath = FileSystem::GetParentDirectory(std::filesystem::path(m_Filepath));

			auto LoadMaterialTextureFunc = [&](auto textureType, auto index = 0)
			{
				SharedRef<Texture2D> result = nullptr;

				aiString path;
				if (mat->GetTexture(textureType, index, &path) == AI_SUCCESS)
				{
					const char* pathCStr = path.C_Str();
					std::filesystem::path filepath = std::filesystem::path(pathCStr);
					std::filesystem::path relativePath = directoryPath / filepath;
					if (FileSystem::Exists(relativePath))
					{
						result = Texture2D::Create(relativePath.string());
						return result;
					}
				}

				return result;
			};

			materialProps.AlbedoMap = LoadMaterialTextureFunc(aiTextureType_DIFFUSE, 0);
			if (!materialProps.AlbedoMap)
				materialProps.AlbedoMap = LoadMaterialTextureFunc(aiTextureType_BASE_COLOR, 0);

			materialProps.NormalMap = LoadMaterialTextureFunc(aiTextureType_NORMALS, 0);
			materialProps.MetallicMap = LoadMaterialTextureFunc(aiTextureType_METALNESS, 0);
			materialProps.RoughnessMap = LoadMaterialTextureFunc(aiTextureType_REFLECTION, 0);
			materialProps.EmissionMap = LoadMaterialTextureFunc(aiTextureType_EMISSIVE, 0);
			materialProps.AmbientOcclusionMap = LoadMaterialTextureFunc(aiTextureType_AMBIENT_OCCLUSION, 0);
			
			material = Material::Create(Renderer::GetShaderLibrary()->Get("PBR_Static"), materialProps);
		}

		return { meshName, vertices, indices, material };
	}

	void StaticMesh::CreateBoundingBoxFromSubmeshes()
	{
		// initialize bounding box to default
		const auto& firstBoundingBox = m_Submeshes.at(0).GetBoundingBox();
		m_BoundingBox = firstBoundingBox;

		for (const auto& submesh : m_Submeshes)
		{
			const Math::AABB& boundingBox = submesh.GetBoundingBox();

			for (uint32_t i = 0; i < 3; i++)
			{
				if (boundingBox.Min[i] < m_BoundingBox.Min[i])
					m_BoundingBox.Min[i] = boundingBox.Min[i];

				if (boundingBox.Max[i] > m_BoundingBox.Max[i])
					m_BoundingBox.Max[i] = boundingBox.Max[i];
			}
		}
	}

	void StaticMesh::OnUpdate(int entityID)
	{
		bool isDirty = false;

		for (auto& submesh : m_Submeshes)
		{
			std::vector<StaticVertex>& vertices = submesh.GetVertices();
			
			size_t dataSize = vertices.size();
			for (uint32_t i = 0; i < dataSize; i++)
			{
				StaticVertex& vertex = vertices[i];
				SharedRef<Material> material = submesh.GetMaterial();

				isDirty = vertex.TexScale != material->GetUV();
				if (!isDirty)
					break;

				vertex.TexScale = material->GetUV();
			}

			if (!isDirty)
				break;

			SharedRef<VertexBuffer> vertexBuffer = submesh.GetVertexBuffer();
			vertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(StaticVertex));
		}
	}

	const StaticSubmesh& StaticMesh::GetSubmesh(uint32_t index) const
	{
		VX_CORE_ASSERT(index < m_Submeshes.size(), "Index out of bounds!");
		return m_Submeshes[index];
	}

	StaticSubmesh& StaticMesh::GetSubmesh(uint32_t index)
	{
		VX_CORE_ASSERT(index < m_Submeshes.size(), "Index out of bounds!");
		return m_Submeshes[index];
	}

	SharedRef<StaticMesh> StaticMesh::Create(StaticMesh::Default defaultMesh, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID)
	{
		return CreateShared<StaticMesh>(defaultMesh, transform, importOptions, (int)(entt::entity)entityID);
	}

	SharedRef<StaticMesh> StaticMesh::Create(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID)
	{
		return CreateShared<StaticMesh>(filepath, transform, importOptions, (int)(entt::entity)entityID);
	}

	SharedRef<StaticMesh> StaticMesh::Create(const std::vector<StaticVertex>& vertices, const std::vector<Index>& indices, const Math::mat4& transform)
	{
		return CreateShared<StaticMesh>(vertices, indices, transform);
	}

	SharedRef<StaticMesh> StaticMesh::Create(MeshType meshType)
	{
		return CreateShared<StaticMesh>(meshType);
	}

}
