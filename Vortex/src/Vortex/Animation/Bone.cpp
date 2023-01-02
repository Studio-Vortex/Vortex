#include "vxpch.h"
#include "Bone.h"

#include "Vortex/Animation/AssimpAPIHelpers.h"

#include <assimp/anim.h>

namespace Vortex {

	Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
		: m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
	{
		m_NumPositions = channel->mNumPositionKeys;
		for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
		{
			KeyPosition data{};
			data.Position = FromAssimpVec3(channel->mPositionKeys[positionIndex].mValue);
			data.TimeStamp = channel->mPositionKeys[positionIndex].mTime;
			m_Positions.push_back(data);
		}

		m_NumRotations = channel->mNumRotationKeys;
		for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
		{
			KeyRotation data{};
			data.Orientation = FromAssimpQuat(channel->mRotationKeys[rotationIndex].mValue);
			data.TimeStamp = channel->mRotationKeys[rotationIndex].mTime;
			m_Rotations.push_back(data);
		}

		m_NumScalings = channel->mNumScalingKeys;
		for (int scaleIndex = 0; scaleIndex < m_NumScalings; ++scaleIndex)
		{
			KeyScale data{};
			data.Scale = FromAssimpVec3(channel->mScalingKeys[scaleIndex].mValue);
			data.TimeStamp = channel->mScalingKeys[scaleIndex].mTime;
			m_Scales.push_back(data);
		}
	}

	void Bone::Update(float animationTime)
	{
		Math::mat4 translation = InterpolatePosition(animationTime);
		Math::mat4 rotation = InterpolateRotation(animationTime);
		Math::mat4 scale = InterpolateScaling(animationTime);
		m_LocalTransform = translation * rotation * scale;
	}

	int Bone::GetPositionIndex(float animationTime) const
	{
		for (int index = 0; index < m_NumPositions - 1; ++index)
		{
			if (animationTime < m_Positions[index + 1].TimeStamp)
			{
				return index;
			}
		}

		VX_CORE_ASSERT(false, "Unknown index!");
		return 0;
	}

	int Bone::GetRotationIndex(float animationTime) const
	{
		for (int index = 0; index < m_NumRotations - 1; ++index)
		{
			if (animationTime < m_Rotations[index + 1].TimeStamp)
			{
				return index;
			}
		}

		VX_CORE_ASSERT(false, "Unknown index!");
		return 0;
	}

	int Bone::GetScaleIndex(float animationTime) const
	{
		for (int index = 0; index < m_NumScalings - 1; ++index)
		{
			if (animationTime < m_Scales[index + 1].TimeStamp)
			{
				return index;
			}
		}

		VX_CORE_ASSERT(false, "Unknown index!");
		return 0;
	}

	float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const
	{
		float scaleFactor = 0.0f;
		float midWayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	Math::mat4 Bone::InterpolatePosition(float animationTime) const
	{
		if (m_NumPositions == 1)
		{
			return Math::Translate(m_Positions[0].Position);
		}

		int p0Index = GetPositionIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Positions[p0Index].TimeStamp, m_Positions[p1Index].TimeStamp, animationTime);
		Math::vec3 finalPosition = Math::Mix(m_Positions[p0Index].Position, m_Positions[p1Index].Position, scaleFactor);
		return Math::Translate(finalPosition);
	}

	Math::mat4 Bone::InterpolateRotation(float animationTime) const
	{
		if (m_NumRotations == 1)
		{
			auto rotation = Math::Normalize(m_Rotations[0].Orientation);
			return Math::ToMat4(rotation);
		}

		int p0Index = GetRotationIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Rotations[p0Index].TimeStamp, m_Rotations[p1Index].TimeStamp, animationTime);
		Math::quaternion finalRotation = Math::Slerp(m_Rotations[p0Index].Orientation, m_Rotations[p1Index].Orientation, scaleFactor);
		finalRotation = Math::Normalize(finalRotation);
		return Math::ToMat4(finalRotation);
	}

	Math::mat4 Bone::InterpolateScaling(float animationTime) const
	{
		if (m_NumScalings == 1)
		{
			return Math::Scale(m_Scales[0].Scale);
		}

		int p0Index = GetScaleIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Scales[p0Index].TimeStamp, m_Scales[p1Index].TimeStamp, animationTime);
		Math::vec3 finalScale = Math::Mix(m_Scales[p0Index].Scale, m_Scales[p1Index].Scale, scaleFactor);
		return Math::Scale(finalScale);
	}

}
