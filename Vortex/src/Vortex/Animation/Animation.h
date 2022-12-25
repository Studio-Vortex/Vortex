#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math.h"
#include "Vortex/Animation/Bone.h"
#include "Vortex/Renderer/Model.h"

#include <string>
#include <unordered_map>

struct aiAnimation;
struct aiNode;

namespace Vortex {

	struct AssimpNodeData
	{
		Math::mat4 Transformation;
		std::string Name;
		int ChildrenCount;
		std::vector<AssimpNodeData> Children;
	};

	class Animation
	{
	public:
		Animation() = default;
		Animation(const std::string& animationPath, const SharedRef<Model>& model);
		~Animation() = default;

		Bone* FindBone(const std::string& name);

		inline float GetTicksPerSecond() const { return m_TicksPerSecond; }
		inline float GetDuration() const { return m_Duration; }
		inline const AssimpNodeData& GetRootNode() const { return m_RootNode; }

		inline std::unordered_map<std::string, BoneInfo>& GetBoneIDMap()
		{
			return m_BoneInfoMap;
		}

		inline const std::string& GetPath() const { return m_Filepath; }

		static SharedRef<Animation> Create(const std::string& animationPath, const SharedRef<Model>& model);

	private:
		void ReadMissingBones(const aiAnimation* animation, const SharedRef<Model>& model);
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
