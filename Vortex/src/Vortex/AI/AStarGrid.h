#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"
#include "Vortex/AI/AStarNode.h"

#include <vector>

namespace Vortex {

	class VORTEX_API AStarGrid
	{
	public:
		AStarGrid();
		~AStarGrid() = default;

	private:
		void CreateGrid();

	private:
		Math::vec2 m_GridWorldSize;
		float m_NodeRadius;
		float m_NodeDiameter;
		uint32_t m_GridSizeX;
		uint32_t m_GridSizeY;
		std::vector<AStarNode> m_Grid;
	};

}
