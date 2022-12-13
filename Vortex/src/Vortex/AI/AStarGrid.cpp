#include "vxpch.h"
#include "AStarGrid.h"

#include "vxpch.h"
#include "AStarGrid.h"

namespace Vortex {

	AStarGrid::AStarGrid()
	{
		m_NodeDiameter = m_NodeRadius * 2.0f;
		m_GridSizeX = (uint32_t)ceil(m_GridWorldSize.x / m_NodeDiameter);
		m_GridSizeY = (uint32_t)ceil(m_GridWorldSize.y / m_NodeDiameter);
		CreateGrid();
	}

	void AStarGrid::CreateGrid()
	{
		m_Grid.reserve(m_GridSizeX * m_GridSizeY);
		Math::vec3 worldBottomLeft = Math::vec3(0.0f) - Math::vec3(1, 0, 0) * m_GridWorldSize.x / 2.0f - Math::vec3(0, 0, -1) * m_GridWorldSize.y / 2.0f;
		
		for (uint32_t y = 0; y < m_GridSizeY; y++)
		{
			for (uint32_t x = 0; x < m_GridSizeX; x++)
			{
				Math::vec3 worldPosition = worldBottomLeft + Math::vec3(1, 0, 0) * (x * m_NodeDiameter + m_NodeRadius) + Math::vec3(0, 0, -1) * (y * m_NodeDiameter + m_NodeRadius);
				//bool walkable = 
			}
		}
	}

}
