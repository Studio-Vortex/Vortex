#pragma once

#include "Vortex/Math/Math.h"

struct aiNodeAnim;

namespace Vortex {

	struct VORTEX_API KeyPosition
	{
		Math::vec3 Position;
		float TimeStamp;
	};

	struct VORTEX_API KeyRotation
	{
		Math::quaternion Orientation;
		float TimeStamp;
	};

	struct VORTEX_API KeyScale
	{
		Math::vec3 Scale;
		float TimeStamp;
	};

	class VORTEX_API Bone
	{
	public:
		/*reads keyframes from aiNodeAnim*/
		Bone(const std::string& name, int ID, const aiNodeAnim* channel);

		/*interpolates  b/w positions,rotations & scaling keys based on the curren time of
		the animation and prepares the local transformation matrix by combining all keys
		tranformations*/
		void Update(float animationTime);

		inline Math::mat4 GetLocalTransform() const { return m_LocalTransform; }
		inline std::string GetBoneName() const { return m_Name; }
		inline int GetBoneID() const { return m_ID; }

		/* Gets the current index on mKeyPositions to interpolate to based on
		the current animation time*/
		int GetPositionIndex(float animationTime) const;

		/* Gets the current index on mKeyRotations to interpolate to based on the
		current animation time*/
		int GetRotationIndex(float animationTime) const;

		/* Gets the current index on mKeyScalings to interpolate to based on the
		current animation time */
		int GetScaleIndex(float animationTime) const;

	private:
		/* Gets normalized value for Lerp & Slerp*/
		float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const;

		/*figures out which position keys to interpolate b/w and performs the interpolation
		and returns the translation matrix*/
		Math::mat4 InterpolatePosition(float animationTime) const;

		/*figures out which rotations keys to interpolate b/w and performs the interpolation
		and returns the rotation matrix*/
		Math::mat4 InterpolateRotation(float animationTime) const;

		/*figures out which scaling keys to interpolate b/w and performs the interpolation
		and returns the scale matrix*/
		Math::mat4 InterpolateScaling(float animationTime) const;

	private:
		std::vector<KeyPosition> m_Positions;
		std::vector<KeyRotation> m_Rotations;
		std::vector<KeyScale> m_Scales;
		int m_NumPositions;
		int m_NumRotations;
		int m_NumScalings;

		Math::mat4 m_LocalTransform;
		std::string m_Name;
		int m_ID;
	};

}
