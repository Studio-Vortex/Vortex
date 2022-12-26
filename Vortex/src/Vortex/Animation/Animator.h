#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math.h"

namespace Vortex {

	class Animation;
	struct AssimpNodeData;

	class Animator
	{
	public:
		Animator(const SharedRef<Animation>& animation);

		void UpdateAnimation(float dt);
		void PlayAnimation();
		void PlayAnimation(const SharedRef<Animation>& animation);
		void Stop();
		void CalculateBoneTransform(const AssimpNodeData* node, Math::mat4 parentTransform);

		inline const std::vector<Math::mat4>& GetFinalBoneMatrices() const { return m_FinalBoneMatrices; }

		bool IsPlaying() const { return m_IsPlaying; }

		static SharedRef<Animator> Create(const SharedRef<Animation>& animation);

	private:
		std::vector<Math::mat4> m_FinalBoneMatrices;
		SharedRef<Animation> m_CurrentAnimation;
		float m_CurrentTime;
		float m_DeltaTime;
		bool m_IsPlaying = false;
	};

}
