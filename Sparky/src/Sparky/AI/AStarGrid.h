#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Core/Math.h"
#include "Sparky/AI/AStarNode.h"

#include <vector>

namespace Sparky {

	class SPARKY_API AStarGrid
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
