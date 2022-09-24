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
	
	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->FindEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity.GetUUID();
	}

	static MonoObject* Entity_GetScriptInstance(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		return ScriptEngine::GetManagedInstance(entityUUID);
	}

	static void Entity_Destroy(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");
		contextScene->DestroyEntity(entity, true);
	}

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

	static void TransformComponent_GetRotation(UUID entityUUID, Math::vec3* outRotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRotation = entity.GetComponent<TransformComponent>().Rotation;
	}

	static void TransformComponent_SetRotation(UUID entityUUID, Math::vec3* rotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TransformComponent>().Rotation = *rotation;
	}

	static void TransformComponent_GetScale(UUID entityUUID, Math::vec3* outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	static void TransformComponent_SetScale(UUID entityUUID, Math::vec3* scale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<TransformComponent>().Scale = *scale;
	}

	static void SpriteComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<SpriteComponent>().SpriteColor;
	}

	static void SpriteComponent_SetColor(UUID entityUUID, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteComponent>().SpriteColor = *color;
	}

	static void SpriteComponent_GetScale(UUID entityUUID, float outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outScale = entity.GetComponent<SpriteComponent>().Scale;
	}

	static void SpriteComponent_SetScale(UUID entityUUID, float scale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteComponent>().Scale = scale;
	}

	static void CircleRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<CircleRendererComponent>().Color;
	}

	static void CircleRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleRendererComponent>().Color = *color;
	}

	static void CircleRendererComponent_GetThickness(UUID entityUUID, float outThickness)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outThickness = entity.GetComponent<CircleRendererComponent>().Thickness;
	}

	static void CircleRendererComponent_SetThickness(UUID entityUUID, float thickness)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleRendererComponent>().Thickness = thickness;
	}

	static void CircleRendererComponent_GetFade(UUID entityUUID, float outFade)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outFade = entity.GetComponent<CircleRendererComponent>().Fade;
	}

	static void CircleRendererComponent_SetFade(UUID entityUUID, float fade)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleRendererComponent>().Fade = fade;
	}

	static void RigidBody2DComponent_ApplyForce(UUID entityUUID, Math::vec2* force, Math::vec2* point, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyForce(b2Vec2(force->x, force->y), b2Vec2(point->x, point->y), wake);
	}

	static void RigidBody2DComponent_ApplyForceToCenter(UUID entityUUID, Math::vec2* force, bool wake)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		auto& rb2d = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyForceToCenter(b2Vec2(force->x, force->y), wake);
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

	static void BoxCollider2D_GetDensity(UUID entityUUID, float outDensity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outDensity = entity.GetComponent<BoxCollider2DComponent>().Density;
	}

	static void BoxCollider2D_SetDensity(UUID entityUUID, float density)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().Density = density;
	}

	static void BoxCollider2D_GetFriction(UUID entityUUID, float outFriction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outFriction = entity.GetComponent<BoxCollider2DComponent>().Friction;
	}

	static void BoxCollider2D_SetFriction(UUID entityUUID, float friction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().Friction = friction;
	}

	static void BoxCollider2D_GetRestitution(UUID entityUUID, float outRestitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outRestitution = entity.GetComponent<BoxCollider2DComponent>().Restitution;
	}

	static void BoxCollider2D_SetRestitution(UUID entityUUID, float restitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().Restitution = restitution;
	}

	static void BoxCollider2D_GetRestitutionThreshold(UUID entityUUID, float outRestitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outRestitutionThreshold = entity.GetComponent<BoxCollider2DComponent>().RestitutionThreshold;
	}

	static void BoxCollider2D_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().RestitutionThreshold = restitutionThreshold;
	}

	static void CircleCollider2D_GetDensity(UUID entityUUID, float outDensity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outDensity = entity.GetComponent<CircleCollider2DComponent>().Density;
	}

	static void CircleCollider2D_SetDensity(UUID entityUUID, float density)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().Density = density;
	}

	static void CircleCollider2D_GetFriction(UUID entityUUID, float outFriction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outFriction = entity.GetComponent<CircleCollider2DComponent>().Friction;
	}

	static void CircleCollider2D_SetFriction(UUID entityUUID, float friction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().Friction = friction;
	}

	static void CircleCollider2D_GetRestitution(UUID entityUUID, float outRestitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outRestitution = entity.GetComponent<CircleCollider2DComponent>().Restitution;
	}

	static void CircleCollider2D_SetRestitution(UUID entityUUID, float restitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().Restitution = restitution;
	}

	static void CircleCollider2D_GetRestitutionThreshold(UUID entityUUID, float outRestitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outRestitutionThreshold = entity.GetComponent<CircleCollider2DComponent>().RestitutionThreshold;
	}

	static void CircleCollider2D_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().RestitutionThreshold = restitutionThreshold;
	}

	static Math::vec3* Algebra_CrossProductVec3(Math::vec3* left, Math::vec3* right)
	{
		Math::vec3 result = Math::Cross(*left, *right);
		return &result;
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
			pos = structName.find("Component"); // In C# the api doesn't include 'Component' in the names of Components
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

#pragma region Entity
		SP_ADD_INTERNAL_CALL(Entity_HasComponent);
		SP_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		SP_ADD_INTERNAL_CALL(Entity_GetScriptInstance);
		SP_ADD_INTERNAL_CALL(Entity_Destroy);
#pragma endregion

#pragma region Transform Component
		SP_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		SP_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		SP_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		SP_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		SP_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		SP_ADD_INTERNAL_CALL(TransformComponent_SetScale);
#pragma endregion

#pragma region Sprite Component
		SP_ADD_INTERNAL_CALL(SpriteComponent_GetColor);
		SP_ADD_INTERNAL_CALL(SpriteComponent_SetColor);
		SP_ADD_INTERNAL_CALL(SpriteComponent_GetScale);
		SP_ADD_INTERNAL_CALL(SpriteComponent_SetScale);
#pragma endregion

#pragma region Circle Renderer Component
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_GetColor);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_SetColor);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_GetThickness);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_SetThickness);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_GetFade);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_SetFade);
#pragma endregion

#pragma region RigidBody2D Component
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyForce);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyForceToCenter);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulseToCenter);
#pragma endregion

#pragma region BoxCollider2D Component
		SP_ADD_INTERNAL_CALL(BoxCollider2D_GetDensity);
		SP_ADD_INTERNAL_CALL(BoxCollider2D_SetDensity);
		SP_ADD_INTERNAL_CALL(BoxCollider2D_GetFriction);
		SP_ADD_INTERNAL_CALL(BoxCollider2D_SetFriction);
		SP_ADD_INTERNAL_CALL(BoxCollider2D_GetRestitution);
		SP_ADD_INTERNAL_CALL(BoxCollider2D_SetRestitution);
		SP_ADD_INTERNAL_CALL(BoxCollider2D_GetRestitutionThreshold);
		SP_ADD_INTERNAL_CALL(BoxCollider2D_SetRestitutionThreshold);
#pragma endregion

#pragma region CircleCollider2D Component
		SP_ADD_INTERNAL_CALL(CircleCollider2D_GetDensity);
		SP_ADD_INTERNAL_CALL(CircleCollider2D_SetDensity);
		SP_ADD_INTERNAL_CALL(CircleCollider2D_GetFriction);
		SP_ADD_INTERNAL_CALL(CircleCollider2D_SetFriction);
		SP_ADD_INTERNAL_CALL(CircleCollider2D_GetRestitution);
		SP_ADD_INTERNAL_CALL(CircleCollider2D_SetRestitution);
		SP_ADD_INTERNAL_CALL(CircleCollider2D_GetRestitutionThreshold);
		SP_ADD_INTERNAL_CALL(CircleCollider2D_SetRestitutionThreshold);
#pragma endregion

#pragma region Algebra
		SP_ADD_INTERNAL_CALL(Algebra_CrossProductVec3);
#pragma endregion

#pragma region Input
		SP_ADD_INTERNAL_CALL(Input_IsKeyDown);
		SP_ADD_INTERNAL_CALL(Input_IsKeyUp);
#pragma endregion

	}

}