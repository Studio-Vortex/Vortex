#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"

namespace Vortex {

	class VORTEX_API AStarNode
	{
	public:
		AStarNode(const Math::vec3& worldPosition, bool walkable);
		~AStarNode() = default;

		inline bool operator==(const AStarNode& other)
		{
			return m_WorldPosition == other.m_WorldPosition && m_Walkable == other.m_Walkable;
		}

	private:
		bool m_Walkable;
		Math::vec3 m_WorldPosition;
	};

}

