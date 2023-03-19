#include "vxpch.h"
#include "StaticMesh.h"

#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Project/Project.h"

#include "Vortex/Asset/AssetManager.h"

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

	StaticSubmesh::StaticSubmesh(const std::string& name, const std::vector<StaticVertex>& vertices, const std::vector<uint32_t>& indices, AssetHandle materialHandle)
		: m_MeshName(name), m_Vertices(vertices), m_Indices(indices), m_MaterialHandle(materialHandle)
	{
		CreateAndUploadMesh();
	}

	// TODO MeshFactory
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

	void StaticSubmesh::SetMaterial(AssetHandle materialHandle)
	{
		m_MaterialHandle = materialHandle;
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

	void StaticSubmesh::Render()
	{
		if (!AssetManager::IsHandleValid(m_MaterialHandle))
			return;

		SharedReference<Material> material = AssetManager::GetAsset<Material>(m_MaterialHandle);
		SharedReference<Shader> shader = material->GetShader();
		material->Bind();

		Renderer::DrawIndexed(shader, m_VertexArray);

		uint32_t triangleCount = m_IndexBuffer->GetCount() / 3;
		Renderer::AddToQuadCountStats(triangleCount / 2);
	}

	void StaticSubmesh::RenderToSkylightShadowMap()
	{
		SharedReference<Shader> shader = Renderer::GetShaderLibrary().Get("SkyLightShadowMap");

		Renderer::DrawIndexed(shader, m_VertexArray);
	}

	StaticMesh::StaticMesh(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID)
		: m_ImportOptions(importOptions)
	{
		LogStream::Initialize();

		VX_CORE_INFO_TAG("Mesh", "Loading Mesh: {}", filepath.c_str());

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath, s_MeshImportFlags);

		if (!scene || !scene->HasMeshes())
		{
			VX_CORE_ERROR_TAG("Mesh", "Failed to load Mesh from: {}", filepath.c_str());
			return;
		}

		m_Scene = scene;

		ProcessNode(filepath, m_Scene->mRootNode, m_Scene, importOptions, entityID);
		CreateBoundingBoxFromSubmeshes();

		m_IsLoaded = true;
	}

	StaticMesh::StaticMesh(MeshType meshType)
	{
		m_Submeshes.push_back(StaticSubmesh(true));
	}

	void StaticMesh::ProcessNode(const std::string& filepath, aiNode* node, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID)
	{
		// process all node meshes
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Submeshes.push_back(ProcessMesh(filepath, mesh, scene, importOptions, entityID));
		}

		// do the same for children nodes
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(filepath, node->mChildren[i], scene, importOptions, entityID);
		}
	}

	StaticSubmesh StaticMesh::ProcessMesh(const std::string& filepath, aiMesh* mesh, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID)
	{
		std::vector<StaticVertex> vertices;
		std::vector<uint32_t> indices;
		AssetHandle materialHandle = 0;

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

			std::filesystem::path directoryPath = FileSystem::GetParentDirectory(std::filesystem::path(filepath));

			auto LoadMaterialTextureFunc = [&](auto textureType, auto index = 0)
			{
				AssetHandle result = 0;

				aiString textureFilepath;

				if (mat->GetTexture(textureType, index, &textureFilepath) != AI_SUCCESS)
					return result;

				const char* pathCStr = textureFilepath.C_Str();
				std::filesystem::path filepath = std::filesystem::path(pathCStr);
				std::filesystem::path relativePath = directoryPath / filepath;

				if (FileSystem::Exists(relativePath))
				{
					result = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(relativePath);
				}

				return result;
			};

			materialProps.AlbedoMap = LoadMaterialTextureFunc(aiTextureType_DIFFUSE, 0);
			materialProps.NormalMap = LoadMaterialTextureFunc(aiTextureType_NORMALS, 0);
			materialProps.MetallicMap = LoadMaterialTextureFunc(aiTextureType_METALNESS, 0);
			materialProps.RoughnessMap = LoadMaterialTextureFunc(aiTextureType_REFLECTION, 0);
			materialProps.EmissionMap = LoadMaterialTextureFunc(aiTextureType_EMISSIVE, 0);
			materialProps.AmbientOcclusionMap = LoadMaterialTextureFunc(aiTextureType_AMBIENT_OCCLUSION, 0);

			SharedReference<Material> material = Material::Create(Renderer::GetShaderLibrary().Get("PBR_Static"), materialProps);

			// This is temporary until we have material files
			Project::GetEditorAssetManager()->AddMemoryOnlyAsset(material);
			materialHandle = material->Handle;
		}

		return { meshName, vertices, indices, materialHandle };
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
		bool dirty = false;

		for (auto& submesh : m_Submeshes)
		{
			std::vector<StaticVertex>& vertices = submesh.GetVertices();
			
			size_t dataSize = vertices.size();
			for (uint32_t i = 0; i < dataSize; i++)
			{
				if (!AssetManager::IsHandleValid(submesh.GetMaterial()))
					continue;

				SharedReference<Material> material = AssetManager::GetAsset<Material>(submesh.GetMaterial());
				if (!material)
					continue;

				StaticVertex& vertex = vertices[i];

				dirty = vertex.TexScale != material->GetUV() || vertex.EntityID != entityID;
				if (!dirty)
					continue;

				vertex.TexScale = material->GetUV();
				vertex.EntityID = entityID;
			}

			if (!dirty)
				continue;

			SharedReference<VertexBuffer> vertexBuffer = submesh.GetVertexBuffer();
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

	SharedReference<StaticMesh> StaticMesh::Create(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID)
	{
		return SharedReference<StaticMesh>::Create(filepath, transform, importOptions, (int)(entt::entity)entityID);
	}

	// TODO put this in meshFactory
	SharedReference<StaticMesh> StaticMesh::Create(MeshType meshType)
	{
		return SharedReference<StaticMesh>::Create(meshType);
	}

}
