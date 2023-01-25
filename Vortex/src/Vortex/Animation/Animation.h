#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math.h"
#include "Vortex/Animation/Bone.h"
#include "Vortex/Renderer/Model.h"
#include "Vortex/Asset/Asset.h"

#include <string>
#include <unordered_map>

struct aiAnimation;
struct aiNode;

namespace Vortex {

	struct VORTEX_API AssimpNodeData
	{
		Math::mat4 Transformation;
		std::string Name;
		int ChildrenCount;
		std::vector<AssimpNodeData> Children;
	};

	class VORTEX_API Animation : public Asset
	{
	public:
		Animation() = default;
		Animation(const std::string& animationPath, SharedRef<Model>& model);
		~Animation() override = default;

		Bone* FindBone(const std::string& name);

		inline float GetTicksPerSecond() const { return (float)m_TicksPerSecond; }
		inline float GetDuration() const { return m_Duration; }
		inline const AssimpNodeData& GetRootNode() const { return m_RootNode; }

		inline std::unordered_map<std::string, BoneInfo>& GetBoneIDMap()
		{
			return m_BoneInfoMap;
		}

		inline const std::string& GetPath() const { return m_Filepath; }

		static AssetType GetStaticType() { return AssetType::Animation; }
		AssetType GetAssetType() const override { return AssetType::Animation; }

		static SharedRef<Animation> Create(const std::string& animationPath, SharedRef<Model>& model);

	private:
		void ReadMissingBones(const aiAnimation* animation, SharedRef<Model>& model);
		void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) const;

	private:
		std::string m_Filepath;
		float m_Duration;
		int m_TicksPerSecond;
		std::vector<Bone> m_Bones;
		AssimpNodeData m_RootNode;
		std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
	};

}
