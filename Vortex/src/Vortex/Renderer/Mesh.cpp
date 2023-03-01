#include "vxpch.h"
#include "Mesh.h"

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

	Submesh::Submesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const SharedRef<Material>& material)
		: m_MeshName(name), m_Vertices(vertices), m_Indices(indices), m_Material(material)
	{
		CreateAndUploadMesh();
	}

	void Submesh::SetMaterial(const SharedRef<Material>& material)
	{
		m_Material = material;
	}

	void Submesh::CreateAndUploadMesh()
	{
		m_VertexArray = VertexArray::Create();

		uint32_t dataSize = m_Vertices.size() * sizeof(Vertex);
		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), dataSize);

		m_VertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"    },
			{ ShaderDataType::Float4, "a_Color"       },
			{ ShaderDataType::Float3, "a_Normal"      },
			{ ShaderDataType::Float3, "a_Tangent"     },
			{ ShaderDataType::Float3, "a_BiTangent"   },
			{ ShaderDataType::Float2, "a_TexCoord"    },
			{ ShaderDataType::Float2, "a_TexScale"    },
			{ ShaderDataType::Int4,   "a_BoneIDs"     },
			{ ShaderDataType::Float4, "a_BoneWeights" },
			{ ShaderDataType::Int,    "a_EntityID"    },
		});

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		CreateBoundingBoxFromVertices();
	}

	void Submesh::CreateBoundingBoxFromVertices()
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

	void Submesh::Render() const
	{
		SharedRef<Shader> shader = m_Material->GetShader();
		m_Material->Bind();

		Renderer::DrawIndexed(shader, m_VertexArray);

		uint32_t triangleCount = m_IndexBuffer->GetCount() / 3;
		Renderer::AddToQuadCountStats(triangleCount / 2);
	}

	void Submesh::RenderToSkylightShadowMap()
	{
		SharedRef<Shader> shader = Renderer::GetShaderLibrary()->Get("SkyLightShadowMap");

		Renderer::DrawIndexed(shader, m_VertexArray);
	}

	Mesh::Mesh(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID)
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

	void Mesh::ProcessNode(aiNode* node, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID)
	{
		// process all node meshes
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Submesh = ProcessMesh(mesh, scene, importOptions, entityID);
		}

		// do the same for children nodes
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, importOptions, entityID);
		}
	}

	Submesh Mesh::ProcessMesh(aiMesh* mesh, const aiScene* scene, const MeshImportOptions& importOptions, const int entityID)
	{
		std::vector<Vertex> vertices;
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
			Vertex vertex;

			SetVertexBoneDataToDefault(vertex);

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
						TextureProperties imageProps;
						imageProps.Filepath = relativePath.string();
						imageProps.WrapMode = ImageWrap::Repeat;

						result = Texture2D::Create(imageProps);
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
			
			material = Material::Create(Renderer::GetShaderLibrary()->Get("PBR"), materialProps);
		}

		m_HasAnimations = ExtractBoneWeightsForVertices(vertices, mesh, scene);

		return { meshName, vertices, indices, material };
	}

	void Mesh::CreateBoundingBoxFromSubmeshes()
	{
		// initialize bounding box to default
		m_BoundingBox = m_Submesh.GetBoundingBox();
	}

	void Mesh::SetVertexBoneDataToDefault(Vertex& vertex) const
	{
		for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.BoneIDs[i] = -1;
			vertex.BoneWeights[i] = 0.0f;
		}
	}

	void Mesh::SetVertexBoneData(Vertex& vertex, int boneID, float weight) const
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			if (vertex.BoneIDs[i] < 0)
			{
				vertex.BoneIDs[i] = boneID;
				vertex.BoneWeights[i] = weight;
				break;
			}
		}
	}

	bool Mesh::ExtractBoneWeightsForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
		if (!scene->HasAnimations())
			return false;

		auto& boneInfoMap = m_BoneInfoMap;
		uint32_t& boneCount = m_BoneCounter;

		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.ID = boneCount;
				newBoneInfo.OffsetMatrix = FromAssimpMat4(mesh->mBones[boneIndex]->mOffsetMatrix);
				boneInfoMap[boneName] = newBoneInfo;
				boneID = boneCount;
				boneCount++;
			}
			else
			{
				boneID = boneInfoMap[boneName].ID;
			}

			assert(boneID != -1);
			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(vertexId <= vertices.size());
				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}

		return true;
	}

	void Mesh::OnUpdate(int entityID)
	{
		bool isDirty = false;

		std::vector<Vertex>& vertices = m_Submesh.GetVertices();

		size_t dataSize = vertices.size();
		for (uint32_t i = 0; i < dataSize; i++)
		{
			Vertex& vertex = vertices[i];
			SharedRef<Material> material = m_Submesh.GetMaterial();

			isDirty = vertex.TexScale != material->GetUV();
			if (!isDirty)
				break;

			vertex.TexScale = material->GetUV();
		}

		if (!isDirty)
			return;

		SharedRef<VertexBuffer> vertexBuffer = m_Submesh.GetVertexBuffer();
		vertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(Vertex));
	}

	SharedRef<Mesh> Mesh::Create(const std::string& filepath, const TransformComponent& transform, const MeshImportOptions& importOptions, int entityID)
	{
		return CreateShared<Mesh>(filepath, transform, importOptions, (int)(entt::entity)entityID);
	}

}
