#include "sppch.h"
#include "AStarNode.h"

namespace Sparky {

	AStarNode::AStarNode(const Math::vec3& worldPosition, bool walkable)
		: m_Walkable(walkable), m_WorldPosition(worldPosition)
	{

	}

}
