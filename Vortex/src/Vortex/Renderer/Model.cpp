#include "vxpch.h"
#include "Model.h"

#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Renderer.h"

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
		aiProcess_GlobalScale |             // e.g. convert cm to m for fbx import (and other formats where cm is native)
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
			VX_CORE_ERROR("Assimp error: {0}", message);
		}
	};

	Mesh::Mesh(const std::vector<ModelVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<SharedRef<MaterialInstance>>& materials)
		: m_Vertices(vertices), m_Indices(indices), m_Materials(materials)
	{
		CreateAndUploadMesh();
		m_Materials.insert(m_Materials.end(), MaterialInstance::Create());
	}

	void Mesh::SetMaterial(const SharedRef<MaterialInstance>& material)
	{
		m_Materials.clear();
		m_Materials[0] = material;
	}

	void Mesh::CreateAndUploadMesh(bool skybox)
	{
		m_VertexArray = VertexArray::Create();

		uint32_t dataSize = m_Vertices.size() * sizeof(ModelVertex);
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), dataSize);

		if (skybox)
			m_VertexBuffer->SetLayout({ { ShaderDataType::Float3, "a_Position" } });
		else
		{
			m_VertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position"  },
				{ ShaderDataType::Float3, "a_Normal"    },
				{ ShaderDataType::Float3, "a_Tangent"   },
				{ ShaderDataType::Float3, "a_BiTangent" },
				{ ShaderDataType::Float2, "a_TexCoord"  },
				{ ShaderDataType::Float2, "a_TexScale"  },
				{ ShaderDataType::Int,    "a_EntityID"  },
			});
		}

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		if (!m_Indices.empty())
		{
			m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
			m_VertexArray->SetIndexBuffer(m_IndexBuffer);
		}
	}

	void Mesh::Render(const SharedRef<Shader>& shader)
	{
		shader->Enable();

		for (auto& material : m_Materials)
		{
			if (SharedRef<Texture2D> normalMap = material->GetNormalMap())
			{
				normalMap->Bind(0);
				shader->SetInt("u_Material.NormalMap", 0);
				shader->SetBool("u_Material.HasNormalMap", true);
			}
			else
				shader->SetBool("u_Material.HasNormalMap", false);

			if (SharedRef<Texture2D> albedoMap = material->GetAlbedoMap())
			{
				albedoMap->Bind(1);
				shader->SetInt("u_Material.AlbedoMap", 1);
				shader->SetBool("u_Material.HasAlbedoMap", true);
			}
			else
				shader->SetBool("u_Material.HasAlbedoMap", false);

			if (SharedRef<Texture2D> metallicMap = material->GetMetallicMap())
			{
				metallicMap->Bind(2);
				shader->SetInt("u_Material.MetallicMap", 2);
				shader->SetBool("u_Material.HasMetallicMap", true);
			}
			else
				shader->SetBool("u_Material.HasMetallicMap", false);

			if (SharedRef<Texture2D> roughnessMap = material->GetRoughnessMap())
			{
				roughnessMap->Bind(3);
				shader->SetInt("u_Material.RoughnessMap", 3);
				shader->SetBool("u_Material.HasRoughnessMap", true);
			}
			else
				shader->SetBool("u_Material.HasRoughnessMap", false);

			if (SharedRef<Texture2D> aoMap = material->GetAmbientOcclusionMap())
			{
				aoMap->Bind(4);
				shader->SetInt("u_Material.AOMap", 4);
				shader->SetBool("u_Material.HasAOMap", true);
			}
			else
				shader->SetBool("u_Material.HasAOMap", false);
		}

		Renderer::DrawIndexed(shader, m_VertexArray);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// process all node meshes
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh, scene, m_EntityID));
		}

		// do the same for children nodes
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const int entityID)
	{
		std::vector<ModelVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<SharedRef<MaterialInstance>> materials;

		// process vertices
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			ModelVertex vertex;

			VX_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions!");
			VX_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals!");

			vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

			vertex.Tangent = Math::vec3(0.0f);
			vertex.BiTangent = Math::vec3(0.0f);
			if (mesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
				vertex.BiTangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
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

		}

		return { vertices, indices, materials };
	}

	Model::Model(const std::string& filepath, const TransformComponent& transform, int entityID)
		: m_Filepath(filepath)
	{
		LogStream::Initialize();

		VX_CORE_INFO("Loading Mesh: {}", m_Filepath.c_str());

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(m_Filepath, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes())
		{
			VX_CORE_ERROR("Failed to load Mesh from: {}", m_Filepath.c_str());
			return;
		}

		m_Scene = scene;
		m_EntityID = entityID;
		m_MeshShader = Renderer::GetShaderLibrary()->Get("PBR");

		ProcessNode(m_Scene->mRootNode, m_Scene);
	}

	Model::Model(Model::Default defaultMesh, const TransformComponent& transform, int entityID)
	{
		LogStream::Initialize();

		m_Filepath = DefaultMeshSourcePaths[static_cast<uint32_t>(defaultMesh)];

		VX_CORE_INFO("Loading Mesh: {}", m_Filepath.c_str());

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(m_Filepath, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes())
		{
			VX_CORE_ERROR("Failed to load Mesh from: {}", m_Filepath.c_str());
			return;
		}

		m_Scene = scene;
		m_EntityID = entityID;
		m_MeshShader = Renderer::GetShaderLibrary()->Get("PBR");

		ProcessNode(m_Scene->mRootNode, m_Scene);
	}

	Model::Model(MeshType meshType)
	{
		m_Filepath = "Resources/Meshes/Cube.fbx";
		
		LogStream::Initialize();

		VX_CORE_INFO("Loading Mesh: {}", m_Filepath.c_str());

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(m_Filepath, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes())
		{
			VX_CORE_ERROR("Failed to load Mesh from: {}", m_Filepath.c_str());
			return;
		}

		m_Scene = scene;

		m_MeshShader = Renderer::GetShaderLibrary()->Get("PBR");

		ProcessNode(m_Scene->mRootNode, m_Scene);
	}

	void Model::OnUpdate(int entityID, const Math::vec2& textureScale)
	{
		if (m_EntityID == entityID && m_TextureScale == textureScale)
			return;

		m_EntityID = entityID;
		m_TextureScale = textureScale;

		for (auto& mesh : m_Meshes)
		{
			std::vector<ModelVertex>& vertices = mesh.GetVertices();
			
			size_t dataSize = vertices.size();
			for (uint32_t i = 0; i < dataSize; i++)
			{
				ModelVertex& vertex = vertices[i];
				vertex.TexScale = m_TextureScale;
				vertex.EntityID = m_EntityID;
			}

			SharedRef<VertexBuffer> vertexBuffer = mesh.GetVertexBuffer();
			vertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(ModelVertex));
		}
	}

	void Model::Render(const Math::mat4& worldSpaceTransform)
	{
		m_MeshShader->Enable();

		SceneLightDescription lightDesc = Renderer::GetSceneLightDescription();
		m_MeshShader->SetInt("u_SceneProperties.ActiveDirectionalLights", lightDesc.ActiveDirLights);
		m_MeshShader->SetInt("u_SceneProperties.ActivePointLights", lightDesc.ActivePointLights);
		m_MeshShader->SetInt("u_SceneProperties.ActiveSpotLights", lightDesc.ActiveSpotLights);

		m_MeshShader->SetMat4("u_Model", worldSpaceTransform);

		m_MeshShader->SetFloat3("u_Material.Albedo", Math::vec3(1.0f));
		m_MeshShader->SetFloat("u_Material.Metallic", 0.5f);
		m_MeshShader->SetFloat("u_Material.Roughness", 0.5f);

		for (auto& mesh : m_Meshes)
			mesh.Render(m_MeshShader);
	}

	void Model::SetMaterial(const SharedRef<MaterialInstance>& material)
	{
		for (auto& mesh : m_Meshes)
			mesh.SetMaterial(material);
	}

	SharedRef<Model> Model::Create(Model::Default defaultMesh, const TransformComponent& transform, int entityID)
	{
		return CreateShared<Model>(defaultMesh, transform, entityID);
	}

	SharedRef<Model> Model::Create(const std::string& filepath, const TransformComponent& transform, int entityID)
	{
		return CreateShared<Model>(filepath, transform, entityID);
	}

	SharedRef<Model> Model::Create(MeshType meshType)
	{
		return CreateShared<Model>(meshType);
	}

}
