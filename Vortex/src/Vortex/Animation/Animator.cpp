#include "vxpch.h"
#include "Animator.h"

#include "Vortex/Animation/Animation.h"

namespace Vortex {

	Animator::Animator(const SharedRef<Animation>& animation)
		: m_CurrentAnimation(animation), m_CurrentTime(0.0f)
	{
		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
		{
			m_FinalBoneMatrices.push_back(Math::mat4(1.0f));
		}
	}

    void Animator::Stop()
    {
		m_IsPlaying = false;
    }

	void Animator::UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;

		if (m_CurrentAnimation)
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), Math::mat4(1.0f));
		}
	}

	void Animator::PlayAnimation()
	{
		m_CurrentTime = 0.0f;
		m_IsPlaying = true;
	}

	void Animator::PlayAnimation(const SharedRef<Animation>& animation)
	{
		m_CurrentAnimation = animation;
		m_CurrentTime = 0.0f;
		m_IsPlaying = true;
	}

	void Animator::CalculateBoneTransform(const AssimpNodeData* node, Math::mat4 parentTransform)
	{
		std::string nodeName = node->Name;
		Math::mat4 nodeTransform = node->Transformation;

		Bone* bone = m_CurrentAnimation->FindBone(nodeName);

		if (bone)
		{
			bone->Update(m_CurrentTime);
			nodeTransform = bone->GetLocalTransform();
		}

		Math::mat4 globalTransformation = parentTransform * nodeTransform;

		auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		auto it = boneInfoMap.find(nodeName);
		if (it != boneInfoMap.end())
		{
			int index = it->second.ID;
			Math::mat4 offset = it->second.OffsetMatrix;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->ChildrenCount; i++)
			CalculateBoneTransform(&node->Children[i], globalTransformation);
	}

	SharedRef<Animator> Animator::Create(const SharedRef<Animation>& animation)
	{
		return CreateShared<Animator>(animation);
	}

}
