#include "vxpch.h"
#include "Model.h"

#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Project/Project.h"

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
			VX_CORE_ERROR("Assimp error: {0}", message);
		}
	};

	Mesh::Mesh(const std::vector<ModelVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<SharedRef<MaterialInstance>>& materials)
		: m_Vertices(vertices), m_Indices(indices), m_Materials(materials)
	{
		CreateAndUploadMesh();
		m_Materials.insert(m_Materials.end(), MaterialInstance::Create());
	}

	Mesh::Mesh(bool skybox)
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

	void Mesh::SetMaterial(const SharedRef<MaterialInstance>& material)
	{
		m_Materials.clear();
		m_Materials.push_back(material);
	}

	void Mesh::CreateAndUploadMesh()
	{
		m_VertexArray = VertexArray::Create();

		uint32_t dataSize = m_Vertices.size() * sizeof(ModelVertex);
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), dataSize);

		m_VertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"  },
			{ ShaderDataType::Float3, "a_Normal"    },
			{ ShaderDataType::Float3, "a_Tangent"   },
			{ ShaderDataType::Float3, "a_BiTangent" },
			{ ShaderDataType::Float2, "a_TexCoord"  },
			{ ShaderDataType::Float2, "a_TexScale"  },
			{ ShaderDataType::Int,    "a_EntityID"  },
		});

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);
	}

	void Mesh::Render(const SharedRef<Shader>& shader, const SharedRef<Material>& material)
	{
		shader->Enable();
		material->Bind();

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

			vertex.Position = Math::vec3{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z } * 0.5f;
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
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<SharedRef<Texture2D>> albedoMaps = LoadMaterialTextures(material, aiTextureType_BASE_COLOR);
			std::vector<SharedRef<Texture2D>> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS);
			std::vector<SharedRef<Texture2D>> metallicMaps = LoadMaterialTextures(material, aiTextureType_METALNESS);
			std::vector<SharedRef<Texture2D>> roughnessMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS);
			std::vector<SharedRef<Texture2D>> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION);

			size_t maxTextures = std::max({ albedoMaps.size(), normalMaps.size(), metallicMaps.size(), roughnessMaps.size(), aoMaps.size() });

			for (uint32_t i = 0; i < maxTextures; i++)
			{
				SharedRef<MaterialInstance> materialInstance = MaterialInstance::Create();

				if (albedoMaps.size() - 1 > i)
				{
					SharedRef<Texture2D> albedoMap = albedoMaps.at(i);
					materialInstance->SetAlbedoMap(albedoMap);
				}
				if (normalMaps.size() - 1 > i)
				{
					SharedRef<Texture2D> normalMap = normalMaps.at(i);
					materialInstance->SetNormalMap(normalMap);
				}
				if (metallicMaps.size() - 1 > i)
				{
					SharedRef<Texture2D> metallicMap = metallicMaps.at(i);
					materialInstance->SetMetallicMap(metallicMap);
				}
				if (roughnessMaps.size() - 1 > i)
				{
					SharedRef<Texture2D> roughnessMap = roughnessMaps.at(i);
					materialInstance->SetRoughnessMap(roughnessMap);
				}
				if (aoMaps.size() - 1 > i)
				{
					SharedRef<Texture2D> ambientOcclusionMap = aoMaps.at(i);
					materialInstance->SetAmbientOcclusionMap(ambientOcclusionMap);
				}

				materials.push_back(materialInstance);
			}
		}

		return { vertices, indices, materials };
	}

	std::vector<SharedRef<Texture2D>> Model::LoadMaterialTextures(aiMaterial* material, uint32_t textureType)
	{
		aiTextureType type = static_cast<aiTextureType>(textureType);
		std::vector<SharedRef<Texture2D>> textures;

		for (uint32_t i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString str;
			material->GetTexture(type, i, &str);

			const char* cStr = str.C_Str();
			if (!std::filesystem::exists(Project::GetAssetDirectory() / cStr))
			{
				VX_CORE_WARN("Skipping texture, not found {}");
				continue;
			}

			SharedRef<Texture2D> texture = Texture2D::Create(cStr);
			textures.push_back(texture);
		}

		return textures;
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
		m_Material = Material::Create(m_MeshShader, MaterialProperties());

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
		m_Material = Material::Create(m_MeshShader, MaterialProperties());

		ProcessNode(m_Scene->mRootNode, m_Scene);
	}

	Model::Model(MeshType meshType)
	{
		m_Meshes.push_back(Mesh(true));
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
		if (!m_MeshShader)
			m_MeshShader = Renderer::GetShaderLibrary()->Get("PBR");

		m_MeshShader->Enable();

		SceneLightDescription lightDesc = Renderer::GetSceneLightDescription();
		m_MeshShader->SetInt("u_SceneProperties.ActiveDirectionalLights", lightDesc.ActiveDirLights);
		m_MeshShader->SetInt("u_SceneProperties.ActivePointLights", lightDesc.ActivePointLights);
		m_MeshShader->SetInt("u_SceneProperties.ActiveSpotLights", lightDesc.ActiveSpotLights);

		m_MeshShader->SetMat4("u_Model", worldSpaceTransform);

		for (auto& mesh : m_Meshes)
		{
			mesh.Render(m_MeshShader, m_Material);
		}
	}

	void Model::SetMaterial(const SharedRef<MaterialInstance>& material)
	{
		for (auto& mesh : m_Meshes)
			mesh.SetMaterial(material);

		m_Material = material;
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
