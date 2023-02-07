#include "vxpch.h"
#include "Model.h"

#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Project/Project.h"
#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/AssimpAPIHelpers.h"

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

	Submesh::Submesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const SharedRef<Material>& material)
		: m_MeshName(name), m_Vertices(vertices), m_Indices(indices), m_Material(material)
	{
		CreateAndUploadMesh();
	}

	Submesh::Submesh(bool skybox)
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

		uint32_t triangleCount = m_IndexBuffer->GetCount() / 3;
		Renderer::AddToQuadCountStats(triangleCount / 2);

		{
			struct ShadowMapVertex
			{
				Math::vec3 Position;
				Math::ivec4 BoneIDs;
				Math::vec4 BoneWeights;
			};

			std::vector<ShadowMapVertex> shadowMapVertices;

			for (auto& vertex : m_Vertices)
			{
				shadowMapVertices.push_back(ShadowMapVertex{ vertex.Position, vertex.BoneIDs, vertex.BoneWeights });
			}

			m_ShadowMapVertexArray = VertexArray::Create();

			uint32_t dataSize = shadowMapVertices.size() * sizeof(ShadowMapVertex);
			m_ShadowMapVertexBuffer = VertexBuffer::Create(shadowMapVertices.data(), dataSize);

			m_ShadowMapVertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position"    },
				{ ShaderDataType::Int4,   "a_BoneIDs"     },
				{ ShaderDataType::Float4, "a_BoneWeights" },
			});

			m_ShadowMapVertexArray->AddVertexBuffer(m_ShadowMapVertexBuffer);

			m_ShadowMapIndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size());
			m_ShadowMapVertexArray->SetIndexBuffer(m_ShadowMapIndexBuffer);
		}
	}

	void Submesh::Render() const
	{
		SharedRef<Shader> shader = m_Material->GetShader();
		shader->Enable();
		m_Material->Bind();

		Renderer::DrawIndexed(shader, m_VertexArray);
		Renderer::AddToDrawCallCountStats(1);
	}

	void Submesh::RenderToSkylightShadowMap(const Math::mat4& worldSpaceTransform)
	{
		SharedRef<Shader> shader = Renderer::GetShaderLibrary()->Get("SkyLightShadowMap");
		shader->Enable();
		shader->SetBool("u_HasAnimations", false);

		Renderer::DrawIndexed(shader, m_ShadowMapVertexArray);
	}
	
	void Submesh::RenderToSkylightShadowMap(const Math::mat4& worldSpaceTransform, const AnimatorComponent& animatorComponent)
	{
		SharedRef<Shader> shader = Renderer::GetShaderLibrary()->Get("SkyLightShadowMap");
		shader->Enable();
		shader->SetBool("u_HasAnimations", true);

		const std::vector<Math::mat4>& transforms = animatorComponent.Animator->GetFinalBoneMatrices();
		for (uint32_t i = 0; i < transforms.size(); i++)
			shader->SetMat4("u_FinalBoneMatrices[" + std::to_string(i) + "]", transforms[i]);

		Renderer::DrawIndexed(shader, m_ShadowMapVertexArray);
	}

	Model::Model(Model::Default defaultMesh, const TransformComponent& transform, const ModelImportOptions& importOptions, int entityID)
		: m_ImportOptions(importOptions)
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

		ProcessNode(m_Scene->mRootNode, m_Scene, importOptions, entityID);
	}

	Model::Model(const std::string& filepath, const TransformComponent& transform, const ModelImportOptions& importOptions, int entityID)
		: m_ImportOptions(importOptions), m_Filepath(filepath)
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

		ProcessNode(m_Scene->mRootNode, m_Scene, importOptions, entityID);
	}

	Model::Model(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const Math::mat4& transform)
	{
		std::vector<Vertex> verts = { Vertex{} };
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
					Vertex transformedVertex = vertex;
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

			Submesh mesh("UnNamed", verts, inds, mat);
			collectionOfMeshes.push_back(mesh);
		};

		if (vertices.size() > 1 && indices.size() > 1)
		{
			TransformVerticesAndGetIndicesAndCreateMesh();
		}
	}

	Model::Model(MeshType meshType)
	{
		m_Submeshes.push_back(Submesh(true));
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene, const ModelImportOptions& importOptions, const int entityID)
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

	Submesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const ModelImportOptions& importOptions, const int entityID)
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

 			vertex.Normal = Math::vec3(transform * Math::vec4(Math::vec3{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z }, 1.0f));
			vertex.Tangent = Math::vec3(0.0f);
			vertex.BiTangent = Math::vec3(0.0f);
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
			material = Material::Create(Renderer::GetShaderLibrary()->Get("PBR"), MaterialProperties());
			// TODO load materials
		}

		m_HasAnimations = ExtractBoneWeightsForVertices(vertices, mesh, scene);

		return { meshName, vertices, indices, material };
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

	void Model::SetVertexBoneDataToDefault(Vertex& vertex) const
	{
		for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.BoneIDs[i] = -1;
			vertex.BoneWeights[i] = 0.0f;
		}
	}

	void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight) const
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

	bool Model::ExtractBoneWeightsForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
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

	void Model::OnUpdate(int entityID)
	{
		bool isDirty = false;

		for (auto& submesh : m_Submeshes)
		{
			std::vector<Vertex>& vertices = submesh.GetVertices();
			
			size_t dataSize = vertices.size();
			for (uint32_t i = 0; i < dataSize; i++)
			{
				Vertex& vertex = vertices[i];
				SharedRef<Material> material = submesh.GetMaterial();

				isDirty = Math::vec3(vertex.Color) != material->GetAlbedo() || vertex.TexScale != material->GetUV();
				if (!isDirty)
					break;

				vertex.Color = Math::vec4(material->GetAlbedo(), 1.0f);
				vertex.TexScale = material->GetUV();
			}

			if (!isDirty)
				break;

			SharedRef<VertexBuffer> vertexBuffer = submesh.GetVertexBuffer();
			vertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(Vertex));
		}
	}

	const Submesh& Model::GetSubmesh(uint32_t index) const
	{
		VX_CORE_ASSERT(index <= m_Submeshes.size() - 1, "Index out of bounds!");
		return m_Submeshes[index];
	}

	Submesh& Model::GetSubmesh(uint32_t index)
	{
		VX_CORE_ASSERT(index < m_Submeshes.size() - 1, "Index out of bounds!");
		return m_Submeshes[index];
	}

	SharedRef<Model> Model::Create(Model::Default defaultMesh, const TransformComponent& transform, const ModelImportOptions& importOptions, int entityID)
	{
		return CreateShared<Model>(defaultMesh, transform, importOptions, (int)(entt::entity)entityID);
	}

	SharedRef<Model> Model::Create(const std::string& filepath, const TransformComponent& transform, const ModelImportOptions& importOptions, int entityID)
	{
		return CreateShared<Model>(filepath, transform, importOptions, (int)(entt::entity)entityID);
	}

	SharedRef<Model> Model::Create(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const Math::mat4& transform)
	{
		return CreateShared<Model>(vertices, indices, transform);
	}

	SharedRef<Model> Model::Create(MeshType meshType)
	{
		return CreateShared<Model>(meshType);
	}

}
