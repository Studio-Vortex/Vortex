#include "vxpch.h"
#include "Animation.h"

#include "Vortex/Animation/AssimpAPIHelpers.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/anim.h>
#include <assimp/scene.h>

namespace Vortex {

	static const uint32_t s_AnimatedMeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split AnimatedMeshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_JoinIdenticalVertices |
		//aiProcess_GlobalScale |             // e.g. convert cm to m for fbx import (and other formats where cm is native)
		aiProcess_ValidateDataStructure;    // Validation

	Animation::Animation(const std::string& animationPath, SharedRef<Model>& model)
		: m_Filepath(animationPath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, s_AnimatedMeshImportFlags);
		VX_CORE_ASSERT(scene && scene->mRootNode, "Invalid Scene");
		auto animation = scene->mAnimations[0];
		m_Duration = (float)animation->mDuration;
		m_TicksPerSecond = (int)animation->mTicksPerSecond;
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		ReadHeirarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(animation, model);
	}

	Bone* Animation::FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);

		if (iter == m_Bones.end())
			return nullptr;
		else
			return &(*iter);
	}

	void Animation::ReadMissingBones(const aiAnimation* animation, SharedRef<Model>& model)
	{
		int size = animation->mNumChannels;

		auto& boneInfoMap = model->GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		uint32_t& boneCount = model->GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;
			auto it = boneInfoMap.find(boneName);

			if (it == boneInfoMap.end())
			{
				boneInfoMap[boneName].ID = boneCount;
				boneCount++;
			}

			Bone bone(boneName, it->second.ID, channel);
			m_Bones.push_back(bone);
		}

		m_BoneInfoMap = boneInfoMap;
	}

	void Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) const
	{
		VX_CORE_ASSERT(src, "Node was null pointer!");

		dest.Name = src->mName.data;
		dest.Transformation = FromAssimpMat4(src->mTransformation);
		dest.ChildrenCount = src->mNumChildren;

		for (uint32_t i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.Children.push_back(newData);
		}
	}

	SharedRef<Animation> Animation::Create(const std::string& animationPath, SharedRef<Model>& model)
	{
		return SharedRef<Animation>::Create(animationPath, model);
	}

}
