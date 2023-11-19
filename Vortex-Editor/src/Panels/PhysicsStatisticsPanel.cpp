#include "PhysicsStatisticsPanel.h"

#include <Vortex/Physics/3D/PhysXAPIHelpers.h>

#include <PhysX/PxPhysicsAPI.h>

namespace Vortex {
	
	void PhysicsStatisticsPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!IsOpen)
			return;

		Gui::Begin("Physics Statistics", &IsOpen);

		if (Physics::GetPhysicsScene() != nullptr)
		{
			if (Gui::Button("Wake up actors"))
			{
				Physics::WakeUpActors();
			}

			physx::PxSimulationStatistics* simulationStats = Physics::GetSimulationStatistics();

			Gui::Text("Active Constraints: %u", simulationStats->nbActiveConstraints);
			Gui::Text("Active Dynamic Bodies: %u", simulationStats->nbActiveDynamicBodies);
			Gui::Text("Active Kinematic Bodies: %u", simulationStats->nbActiveKinematicBodies);
			Gui::Text("Dynamic Bodies: %u", simulationStats->nbDynamicBodies);
			Gui::Text("Kinematic Bodies: %u", simulationStats->nbKinematicBodies);
			Gui::Text("Static Bodies: %u", simulationStats->nbStaticBodies);
			Gui::Text("Aggregates: %u", simulationStats->nbAggregates);
			Gui::Text("Articulations: %u", simulationStats->nbArticulations);
			Gui::Text("Axis Solver Constraints: %u", simulationStats->nbAxisSolverConstraints);
			Gui::Text("Broadphase Volume Adds: %u", simulationStats->getNbBroadPhaseAdds());
			Gui::Text("Broadphase Volume Removes: %u", simulationStats->getNbBroadPhaseRemoves());
			Gui::Text("Discrete Contact Pairs Total: %u", simulationStats->nbDiscreteContactPairsTotal);
			Gui::Text("Discrete Contact Pairs w/ Cache Hits: %u", simulationStats->nbDiscreteContactPairsWithCacheHits);
			Gui::Text("Discrete Contact Pairs w/ Contacts: %u", simulationStats->nbDiscreteContactPairsWithContacts);
			Gui::Text("Lost Pairs: %u", simulationStats->nbLostPairs);
			Gui::Text("Lost Touches: %u", simulationStats->nbLostTouches);
			Gui::Text("New Pairs: %u", simulationStats->nbNewPairs);
			Gui::Text("New Touches: %u", simulationStats->nbNewTouches);
			Gui::Text("Partitions: %u", simulationStats->nbPartitions);

			for (uint32_t i = 0; i < physx::PxGeometryType::eGEOMETRY_COUNT; i++)
				Gui::Text("%s Shapes: %u", PhysXGeometryTypeToString((physx::PxGeometryType::Enum)i).c_str(), simulationStats->nbShapes[i]);
		
			Gui::Text("Compressed Contact Size (Bytes): %u", simulationStats->compressedContactSize);
			Gui::Text("Peak Constraint Size (Bytes): %u", simulationStats->peakConstraintMemory);
			Gui::Text("Required Contact Constraint Size (Bytes): %u", simulationStats->requiredContactConstraintMemory);
		}

		Gui::End();
	}

}
