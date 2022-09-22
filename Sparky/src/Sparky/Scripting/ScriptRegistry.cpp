#include "sppch.h"
#include "ScriptRegistry.h"

#include "Sparky/Core/UUID.h"
#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scripting/ScriptEngine.h"

#include "Sparky/Core/MouseCodes.h"
#include "Sparky/Core/KeyCodes.h"
#include "Sparky/Core/Input.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <box2d/b2_body.h>

namespace Sparky {

#define SP_ADD_INTERNAL_CALL(icall) mono_add_internal_call("Sparky.InternalCalls::" #icall, icall)

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

	static void TransformComponent_GetTranslation(UUID entityUUID, Math::vec3* outTranslation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;
	}

	static void TransformComponent_SetTranslation(UUID entityUUID, Math::vec3* translation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	static void RigidBody2DComponent_ApplyLinearImpulse(UUID entityUUID, Math::vec2* impulse, Math::vec2* point, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}
	
	static void RigidBody2DComponent_ApplyLinearImpulseToCenter(UUID entityUUID, Math::vec2* impulse, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

	static bool Entity_HasComponent(UUID entityUUID, MonoReflectionType* componentType)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		SP_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end(), "Managed type was not found in Function Map!");
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	static bool Input_IsKeyDown(KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}
	
	static bool Input_IsKeyUp(KeyCode key)
	{
		return Input::IsKeyReleased(key);
	}

	template <typename... TComponent>
	static void RegisterComponent()
	{
		([]()
		{
			std::string_view typeName = typeid(TComponent).name();
			size_t pos = typeName.find_last_of(':');
			std::string_view structName = typeName.substr(pos + 1);
			pos = structName.find("Component");
			std::string managedTypename = fmt::format("Sparky.{}", structName.substr(0, pos));

			MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());

			if (!managedType)
			{
				SP_CORE_ERROR("Could not find Component type {}", managedTypename);
				return;
			}

			s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<TComponent>(); };
		}(), ...);
	}

	template <typename... TComponent>
	static void RegisterComponent(ComponentGroup<TComponent...>)
	{
		RegisterComponent<TComponent...>();
	}

	void ScriptRegistry::RegisterComponents()
	{
		RegisterComponent(AllComponents{});
	}

	void ScriptRegistry::RegisterMethods()
	{
		SP_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		SP_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulseToCenter);

		SP_ADD_INTERNAL_CALL(Entity_HasComponent);

		SP_ADD_INTERNAL_CALL(Input_IsKeyDown);
		SP_ADD_INTERNAL_CALL(Input_IsKeyUp);
	}

}