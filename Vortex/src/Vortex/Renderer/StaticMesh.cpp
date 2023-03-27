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

	StaticSubmesh::StaticSubmesh(const std::string& name, const std::vector<StaticVertex>& vertices, const std::vector<uint32_t>& indices)
		: m_MeshName(name), m_Vertices(vertices), m_Indices(indices)
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

	void StaticSubmesh::Render(AssetHandle materialHandle) const
	{
		VX_CORE_ASSERT(AssetManager::IsHandleValid(materialHandle), "Invalid Material!");

		SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
		if (!material)
		{
			VX_CORE_ASSERT(false, "Invalid Material!");
			return;
		}

		SharedReference<Shader> shader = material->GetShader();
		material->Bind();

		Renderer::DrawIndexed(shader, m_VertexArray);

		uint32_t triangleCount = m_IndexBuffer->GetCount() / 3;
		Renderer::AddToQuadCountStats(triangleCount / 2);
	}

	void StaticSubmesh::RenderToSkylightShadowMap() const
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

		uint32_t submeshIndex = 0;
		ProcessNode(submeshIndex, filepath, m_Scene->mRootNode, m_Scene, importOptions, entityID);
		CreateBoundingBoxFromSubmeshes();

		m_IsLoaded = true;
	}

	StaticMesh::StaticMesh(MeshType meshType)
	{
		// Create cube from vertices
		m_Submeshes[0] = StaticSubmesh(true);
	}

	void StaticMesh::ProcessNode(uint32_t& submeshIndex, const std::string& filepath, aiNode* node, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID)
	{
		// process all node meshes
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			uint32_t currentSubmesh = submeshIndex;
			m_Submeshes[currentSubmesh] = ProcessMesh(submeshIndex, filepath, mesh, scene, importOptions, entityID);
		}

		// do the same for children nodes
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(submeshIndex, filepath, node->mChildren[i], scene, importOptions, entityID);
		}
	}

	StaticSubmesh StaticMesh::ProcessMesh(uint32_t& submeshIndex, const std::string& filepath, aiMesh* mesh, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID)
	{
		std::vector<StaticVertex> vertices;
		std::vector<uint32_t> indices;

		const TransformComponent& importTransform = importOptions.MeshTransformation;
		Math::vec3 rotation = importTransform.GetRotationEuler();
		Math::mat4 transform = Math::Translate(importTransform.Translation) *
			Math::Rotate(Math::Deg2Rad(rotation.x), { 1.0f, 0.0f, 0.0f }) *
			Math::Rotate(Math::Deg2Rad(rotation.y), { 0.0f, 1.0f, 0.0f }) *
			Math::Rotate(Math::Deg2Rad(rotation.z), { 0.0f, 0.0f, 1.0f }) *
			Math::Scale(importTransform.Scale);

		const char* nameCStr = mesh->mName.C_Str();
		std::string submeshName = std::string(nameCStr);

		// process vertices
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			StaticVertex vertex;

			ProcessVertex(mesh, vertex, transform, i);

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

		std::vector<AssetHandle> materialTextures =
		{
			0, 0, 0, 0, 0, 0,
		};

		// process materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

#ifndef VX_DIST

			m_MaterialNames[submeshIndex] = std::string(material->GetName().C_Str());
			if (m_MaterialNames[submeshIndex].empty())
				m_MaterialNames[submeshIndex] = submeshName;
			if (m_MaterialNames[submeshIndex].size() > 25)
				m_MaterialNames[submeshIndex].erase(25, m_MaterialNames[submeshIndex].size() - 25);

#endif

			std::string directory = FileSystem::GetParentDirectory(filepath).string();

			materialTextures = 
			{
				GetMaterialTexture(material, directory, (uint32_t)aiTextureType_DIFFUSE, 0),
				GetMaterialTexture(material, directory, (uint32_t)aiTextureType_NORMALS, 0),
				GetMaterialTexture(material, directory, (uint32_t)aiTextureType_METALNESS, 0),
				GetMaterialTexture(material, directory, (uint32_t)aiTextureType_REFLECTION, 0),
				GetMaterialTexture(material, directory, (uint32_t)aiTextureType_EMISSIVE, 0),
				GetMaterialTexture(material, directory, (uint32_t)aiTextureType_AMBIENT_OCCLUSION, 0),
			};
		}

		// Get or Create Material
		AssetHandle materialHandle = 0;
		std::string materialName = m_MaterialNames[submeshIndex];

		std::string filename = materialName + ".vmaterial";
		const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata("Materials/" + filename);
		if (AssetManager::IsHandleValid(metadata.Handle))
		{
			materialHandle = metadata.Handle;
			m_MaterialHandles[submeshIndex] = metadata.Handle;
		}
		else
		{
			// Create new asset
			MaterialProperties materialProps;
			materialProps.AlbedoMap = materialTextures[0];
			materialProps.NormalMap = materialTextures[1];
			materialProps.MetallicMap = materialTextures[2];
			materialProps.RoughnessMap = materialTextures[3];
			materialProps.EmissionMap = materialTextures[4];
			materialProps.AmbientOcclusionMap = materialTextures[5];

			SharedReference<Shader> shader = Renderer::GetShaderLibrary().Get("PBR_Static");
			SharedReference<Material> material = Project::GetEditorAssetManager()->CreateNewAsset<Material>("Materials", filename, shader, materialProps);
			VX_CORE_ASSERT(AssetManager::IsHandleValid(material->Handle), "Invalid asset handle!");
			
			material->SetName(materialName);
			materialHandle = material->Handle;
			m_InitialMaterialHandles[submeshIndex] = materialHandle;
		}

		submeshIndex++;

		return { submeshName, vertices, indices };
	}

	void StaticMesh::ProcessVertex(aiMesh* mesh, StaticVertex& vertex, const Math::mat4& transform, uint32_t index)
	{
		VX_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions!");
		VX_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals!");

		vertex.Position = Math::vec3(transform * Math::vec4(Math::vec3{ mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z }, 1.0f)) * 0.5f;

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
			Math::vec3 normal(mesh->mNormals[index].x, mesh->mNormals[index].y, mesh->mNormals[index].z);
			Math::vec4 normalXYZW(normal, 1.0f);
			vertex.Normal = Math::vec3(transform * normalXYZW);
		}

		if (mesh->HasTangentsAndBitangents())
		{
			vertex.Tangent = Math::vec3(transform * Math::vec4(Math::vec3{ mesh->mTangents[index].x, mesh->mTangents[index].y, mesh->mTangents[index].z }, 1.0f));
			vertex.BiTangent = Math::vec3(transform * Math::vec4(Math::vec3{ mesh->mBitangents[index].x, mesh->mBitangents[index].y, mesh->mBitangents[index].z }, 1.0f));
		}

		vertex.TexScale = Math::vec2(1.0f);

		vertex.TexCoord = Math::vec2(0.0f);
		// does it contain texture coords?
		if (mesh->mTextureCoords[0])
		{
			vertex.TexCoord = { mesh->mTextureCoords[0][index].x, mesh->mTextureCoords[0][index].y };
		}
	}

	AssetHandle StaticMesh::GetMaterialTexture(aiMaterial* material, const std::filesystem::path& directory, uint32_t textureType, uint32_t index)
	{
		AssetHandle result = 0;
		aiString textureFilepath;

		if (material->GetTexture((aiTextureType)textureType, index, &textureFilepath) != AI_SUCCESS)
			return result;

		const char* pathCStr = textureFilepath.C_Str();
		std::filesystem::path filepath = std::filesystem::path(pathCStr).filename();
		std::filesystem::path relativePath = directory / filepath;
		std::filesystem::path directoryName = directory.filename();

		if (FileSystem::Exists(relativePath))
		{
			result = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(relativePath);
		}

		if (!AssetManager::IsHandleValid(result))
		{
			std::filesystem::path texturesPath = "Assets/Textures" / filepath;
			if (FileSystem::Exists(texturesPath))
				result = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(texturesPath);

			if (!AssetManager::IsHandleValid(result))
			{
				std::filesystem::path texturesPathWithDirectory = "Assets/Textures" / directoryName / filepath;
				if (FileSystem::Exists(texturesPathWithDirectory))
					result = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(texturesPathWithDirectory);
			}
		}

		return result;
	}

	void StaticMesh::CreateBoundingBoxFromSubmeshes()
	{
		const auto& firstBoundingBox = m_Submeshes.at(0).GetBoundingBox();
		m_BoundingBox = firstBoundingBox;

		for (const auto& [submeshIndex, submesh] : m_Submeshes)
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

	void StaticMesh::OnUpdate(const SharedReference<MaterialTable>& materialTable, int entityID)
	{
		bool dirty = false;

		for (auto& [submeshIndex, submesh] : m_Submeshes)
		{
			VX_CORE_ASSERT(materialTable->HasMaterial(submeshIndex), "Material Table not synchronized with component!");

			AssetHandle materialHandle = materialTable->GetMaterial(submeshIndex);
			if (!AssetManager::IsHandleValid(materialHandle))
				continue;

			SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
			if (!material)
				continue;

			std::vector<StaticVertex>& vertices = submesh.GetVertices();

			size_t dataSize = vertices.size();
			for (uint32_t i = 0; i < dataSize; i++)
			{
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

	void StaticMesh::LoadMaterialTable(SharedReference<MaterialTable>& materialTable)
	{
		uint32_t currentSubmeshIndex = 0;
		uint32_t materialCount = m_InitialMaterialHandles.size() + m_MaterialHandles.size();

		// TODO how should we handle this?
		VX_CORE_ASSERT(materialCount == m_Submeshes.size(), "Size mismatch!");

		const bool sizeMismatch = materialCount != m_Submeshes.size();
		if (sizeMismatch)
		{
			const bool hasUserSetMaterials = !m_MaterialHandles.empty();
			if (hasUserSetMaterials)
			{
				m_InitialMaterialHandles.clear();
				materialCount = m_MaterialHandles.size();
			}
		}

		std::unordered_map<uint32_t, AssetHandle> handles;

		while (currentSubmeshIndex < materialCount)
		{
			AssetHandle initalHandle = m_InitialMaterialHandles[currentSubmeshIndex];
			AssetHandle userSetHandle = m_MaterialHandles[currentSubmeshIndex];

			if (AssetManager::IsHandleValid(initalHandle))
				handles[currentSubmeshIndex] = initalHandle;
			if (AssetManager::IsHandleValid(userSetHandle))
				handles[currentSubmeshIndex] = userSetHandle;

			VX_CORE_ASSERT(handles.contains(currentSubmeshIndex), "Failed to find material for submesh!");

			currentSubmeshIndex++;
		}

		for (const auto& [submeshIndex, materialHandle] : handles)
		{
			materialTable->SetMaterial(submeshIndex, materialHandle);
		}
	}

	bool StaticMesh::HasSubmesh(uint32_t index) const
	{
		return m_Submeshes.contains(index);
	}

	const StaticSubmesh& StaticMesh::GetSubmesh(uint32_t index) const
	{
		VX_CORE_ASSERT(HasSubmesh(index), "Index out of bounds!");
		return m_Submeshes.at(index);
	}

	StaticSubmesh& StaticMesh::GetSubmesh(uint32_t index)
	{
		VX_CORE_ASSERT(HasSubmesh(index), "Index out of bounds!");
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
