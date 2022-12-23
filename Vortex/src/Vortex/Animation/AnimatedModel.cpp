#include "vxpch.h"
#include "AnimatedModel.h"

#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Project/Project.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Vortex {

	static const uint32_t s_AnimatedMeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split AnimatedMeshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeAnimatedMeshes |          // Batch draws where possible
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

	AnimatedMesh::AnimatedMesh(const std::vector<AnimatedModelVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<SharedRef<MaterialInstance>>& materials)
		: m_Vertices(vertices), m_Indices(indices), m_Materials(materials)
	{
		CreateAndUploadAnimatedMesh();
		m_Materials.insert(m_Materials.end(), MaterialInstance::Create());
	}

	AnimatedMesh::AnimatedMesh(bool skybox)
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

	void AnimatedMesh::SetMaterial(const SharedRef<MaterialInstance>& material)
	{
		m_Materials.clear();
		m_Materials.push_back(material);
	}

	void AnimatedMesh::CreateAndUploadAnimatedMesh()
	{
		m_VertexArray = VertexArray::Create();

		uint32_t dataSize = m_Vertices.size() * sizeof(AnimatedModelVertex);
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

		{
			std::vector<Math::vec3> positions;

			for (auto& vertex : m_Vertices)
			{
				positions.push_back(vertex.Position);
			}

			m_ShadowMapVertexArray = VertexArray::Create();

			uint32_t dataSize = positions.size() * sizeof(Math::vec3);
			m_ShadowMapVertexBuffer = VertexBuffer::Create(positions.data(), dataSize);

			m_ShadowMapVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" }
			});

			m_ShadowMapVertexArray->AddVertexBuffer(m_ShadowMapVertexBuffer);

			m_ShadowMapIndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
			m_ShadowMapVertexArray->SetIndexBuffer(m_ShadowMapIndexBuffer);
		}
	}

	void AnimatedMesh::Render(const SharedRef<Shader>& shader, const SharedRef<Material>& material)
	{
		shader->Enable();
		material->Bind();

		Renderer::DrawIndexed(shader, m_VertexArray);
	}

	void AnimatedMesh::RenderForShadowMap(const SharedRef<Shader>& shader, const SharedRef<Material>& material)
	{
		Renderer::DrawIndexed(shader, m_ShadowMapVertexArray);
	}

	AnimatedModel::AnimatedModel(const std::string& filepath, const TransformComponent& transform, const AnimatedModelImportOptions& importOptions, int entityID)
		: m_ImportOptions(importOptions), m_Filepath(filepath)
	{
		LogStream::Initialize();

		VX_CORE_INFO("Loading AnimatedMesh: {}", m_Filepath.c_str());

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(m_Filepath, s_AnimatedMeshImportFlags);
		if (!scene || !scene->HasAnimatedMeshes())
		{
			VX_CORE_ERROR("Failed to load AnimatedMesh from: {}", m_Filepath.c_str());
			return;
		}

		m_Scene = scene;
		m_EntityID = entityID;
		m_AnimatedMeshShader = Renderer::GetShaderLibrary()->Get("PBR");
		m_Material = Material::Create(m_AnimatedMeshShader, MaterialProperties());

		ProcessNode(m_Scene->mRootNode, m_Scene, importOptions);
	}

	void AnimatedModel::ProcessNode(aiNode* node, const aiScene* scene, const AnimatedModelImportOptions& importOptions)
	{
		// process all node AnimatedMeshes
		for (uint32_t i = 0; i < node->mNumAnimatedMeshes; i++)
		{
			aiAnimatedMesh* AnimatedMesh = scene->mAnimatedMeshes[node->mAnimatedMeshes[i]];
			m_AnimatedMeshes.push_back(ProcessAnimatedMesh(AnimatedMesh, scene, importOptions, m_EntityID));
		}

		// do the same for children nodes
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, importOptions);
		}
	}

	AnimatedMesh AnimatedModel::ProcessAnimatedMesh(aiAnimatedMesh* AnimatedMesh, const aiScene* scene, const AnimatedModelImportOptions& importOptions, const int entityID)
	{
		std::vector<AnimatedModelVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<SharedRef<MaterialInstance>> materials;

		const TransformComponent& importTransform = importOptions.AnimatedMeshTransformation;
		Math::vec3 rotation = importTransform.GetRotationEuler();
		Math::mat4 transform = Math::Translate(importTransform.Translation) *
			Math::Rotate(Math::Deg2Rad(rotation.x), { 1.0f, 0.0f, 0.0f }) *
			Math::Rotate(Math::Deg2Rad(rotation.y), { 0.0f, 1.0f, 0.0f }) *
			Math::Rotate(Math::Deg2Rad(rotation.z), { 0.0f, 0.0f, 1.0f }) *
			Math::Scale(importTransform.Scale);

		// process vertices
		for (uint32_t i = 0; i < AnimatedMesh->mNumVertices; i++)
		{
			AnimatedModelVertex vertex;

			VX_CORE_ASSERT(AnimatedMesh->HasPositions(), "AnimatedMeshes require positions!");
			VX_CORE_ASSERT(AnimatedMesh->HasNormals(), "AnimatedMeshes require normals!");

			vertex.Position = Math::vec3(transform * Math::vec4(Math::vec3{ AnimatedMesh->mVertices[i].x, AnimatedMesh->mVertices[i].y, AnimatedMesh->mVertices[i].z }, 1.0f)) * 0.5f;
			vertex.Normal = Math::vec3(transform * Math::vec4(Math::vec3{ AnimatedMesh->mNormals[i].x, AnimatedMesh->mNormals[i].y, AnimatedMesh->mNormals[i].z }, 1.0f));

			vertex.Tangent = Math::vec3(0.0f);
			vertex.BiTangent = Math::vec3(0.0f);
			if (AnimatedMesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = Math::vec3(transform * Math::vec4(Math::vec3{ AnimatedMesh->mTangents[i].x, AnimatedMesh->mTangents[i].y, AnimatedMesh->mTangents[i].z }, 1.0f));
				vertex.BiTangent = Math::vec3(transform * Math::vec4(Math::vec3{ AnimatedMesh->mBitangents[i].x, AnimatedMesh->mBitangents[i].y, AnimatedMesh->mBitangents[i].z }, 1.0f));
			}

			vertex.TexScale = Math::vec2(1.0f);

			vertex.TexCoord = Math::vec2(0.0f);
			// does it contain texture coords?
			if (AnimatedMesh->mTextureCoords[0])
			{
				vertex.TexCoord = { AnimatedMesh->mTextureCoords[0][i].x, AnimatedMesh->mTextureCoords[0][i].y };
			}

			vertex.EntityID = entityID;
			vertices.push_back(vertex);
		}

		// process indices
		for (uint32_t i = 0; i < AnimatedMesh->mNumFaces; i++)
		{
			aiFace face = AnimatedMesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// process materials
		if (AnimatedMesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[AnimatedMesh->mMaterialIndex];
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

	std::vector<SharedRef<Texture2D>> AnimatedModel::LoadMaterialTextures(aiMaterial* material, uint32_t textureType)
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

	void AnimatedModel::OnUpdate(int entityID, const Math::vec2& textureScale)
	{
		if (m_EntityID == entityID && m_TextureScale == textureScale)
			return;

		m_EntityID = entityID;
		m_TextureScale = textureScale;

		for (auto& AnimatedMesh : m_AnimatedMeshes)
		{
			std::vector<AnimatedModelVertex>& vertices = AnimatedMesh.GetVertices();

			size_t dataSize = vertices.size();
			for (uint32_t i = 0; i < dataSize; i++)
			{
				AnimatedModelVertex& vertex = vertices[i];
				vertex.TexScale = m_TextureScale;
				vertex.EntityID = m_EntityID;
			}

			SharedRef<VertexBuffer> vertexBuffer = AnimatedMesh.GetVertexBuffer();
			vertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(AnimatedModelVertex));
		}
	}

	void AnimatedModel::Render(const Math::mat4& worldSpaceTransform)
	{
		m_AnimatedMeshShader = Renderer::GetShaderLibrary()->Get("PBR");

		m_AnimatedMeshShader->Enable();

		SceneLightDescription lightDesc = Renderer::GetSceneLightDescription();
		m_AnimatedMeshShader->SetInt("u_SceneProperties.ActiveDirectionalLights", lightDesc.ActiveDirLights);
		m_AnimatedMeshShader->SetInt("u_SceneProperties.ActivePointLights", lightDesc.ActivePointLights);
		m_AnimatedMeshShader->SetInt("u_SceneProperties.ActiveSpotLights", lightDesc.ActiveSpotLights);

		m_AnimatedMeshShader->SetMat4("u_AnimatedModel", worldSpaceTransform);

		Renderer::BindDepthMap();
		for (auto& AnimatedMesh : m_AnimatedMeshes)
		{
			AnimatedMesh.Render(m_AnimatedMeshShader, m_Material);
		}
	}

	void AnimatedModel::RenderForShadowMap(const Math::mat4& worldSpaceTransform)
	{
		m_AnimatedMeshShader = Renderer::GetShaderLibrary()->Get("ShadowMap");

		for (auto& AnimatedMesh : m_AnimatedMeshes)
		{
			AnimatedMesh.RenderForShadowMap(m_AnimatedMeshShader, m_Material);
		}
	}

	void AnimatedModel::SetMaterial(const SharedRef<MaterialInstance>& material)
	{
		for (auto& AnimatedMesh : m_AnimatedMeshes)
			AnimatedMesh.SetMaterial(material);

		m_Material = material;
	}

	SharedRef<AnimatedModel> AnimatedModel::Create(const std::string& filepath, const TransformComponent& transform, const AnimatedModelImportOptions& importOptions, int entityID)
	{
		return CreateShared<AnimatedModel>(filepath, transform, importOptions, entityID);
	}

}
