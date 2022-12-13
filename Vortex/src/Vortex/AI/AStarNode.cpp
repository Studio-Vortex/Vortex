#include "vxpch.h"
#include "AStarNode.h"

namespace Vortex {

	AStarNode::AStarNode(const Math::vec3& worldPosition, bool walkable)
		: m_Walkable(walkable), m_WorldPosition(worldPosition)
	{

	}

}
