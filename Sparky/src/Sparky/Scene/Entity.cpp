#include "sppch.h"
#include "Entity.h"

namespace Sparky {
	
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityID(handle), m_Scene(scene)
	{
	}

}