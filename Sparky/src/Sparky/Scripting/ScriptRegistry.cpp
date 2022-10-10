#include "sppch.h"
#include "ScriptRegistry.h"

#include "Sparky/Core/Application.h"

#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scripting/ScriptEngine.h"

#include "Sparky/Core/UUID.h"
#include "Sparky/Core/MouseCodes.h"
#include "Sparky/Core/KeyCodes.h"
#include "Sparky/Core/Input.h"

#include "Sparky/Renderer/RenderCommand.h"
#include "Sparky/Renderer/Renderer2D.h"

#include "Sparky/Core/Log.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <box2d/b2_body.h>
#include <box2d/b2_world.h>
#include <box2d/b2_fixture.h>

#include <cstdlib>
#include <ctime>

namespace Sparky {

#define SP_ADD_INTERNAL_CALL(icall) mono_add_internal_call("Sparky.InternalCalls::" #icall, icall)

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

	static Math::vec4 s_RaycastDebugLineColor = Math::vec4(1.0f, 0.0f, 0.0f, 1.0f);

#pragma region Game

	static void Application_Shutdown()
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		Application::Get().Close();
	}

#pragma endregion

#pragma region Entity

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

	static void Entity_AddCamera(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.AddComponent<CameraComponent>();
	}

	static void Entity_RemoveCamera(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.RemoveComponent<CameraComponent>();
	}

	static void Entity_AddSpriteRenderer(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.AddComponent<SpriteRendererComponent>();
	}

	static void Entity_RemoveSpriteRenderer(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.RemoveComponent<SpriteRendererComponent>();
	}
	
	static void Entity_AddCircleRenderer(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.AddComponent<CircleRendererComponent>();
	}

	static void Entity_RemoveCircleRenderer(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.RemoveComponent<CircleRendererComponent>();
	}

	static void Entity_AddRigidBody2D(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.AddComponent<RigidBody2DComponent>();
	}

	static void Entity_RemoveRigidBody2D(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.RemoveComponent<RigidBody2DComponent>();
	}

	static void Entity_AddBoxCollider2D(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.AddComponent<BoxCollider2DComponent>();
	}

	static void Entity_RemoveBoxCollider2D(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.RemoveComponent<BoxCollider2DComponent>();
	}

	static void Entity_AddCircleCollider2D(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.AddComponent<CircleCollider2DComponent>();
	}

	static void Entity_RemoveCircleCollider2D(UUID entityUUID)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.RemoveComponent<CircleCollider2DComponent>();
	}
	
	static void Entity_GetName(UUID entityUUID, MonoString* outEntityName)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const std::string& name = entity.GetName();

		outEntityName = mono_string_new_wrapper(name.c_str());
	}

	static uint64_t Entity_CreateWithName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->CreateEntity(nameCStr);
		mono_free(nameCStr);

		return entity.GetUUID();
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

	static void Entity_Destroy(UUID entityUUID, bool isScriptInstance = true)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		contextScene->DestroyEntity(entity, isScriptInstance);
	}

#pragma endregion

#pragma region Transform Component

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

		// Since we store rotation in radians we must convert to degrees here
		outRotation->x = Math::Rad2Deg(outRotation->x);
		outRotation->y = Math::Rad2Deg(outRotation->y);
		outRotation->z = Math::Rad2Deg(outRotation->z);
	}

	static void TransformComponent_SetRotation(UUID entityUUID, Math::vec3* rotation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		// Since we store rotation in radians we must convert to radians here
		rotation->x = Math::Deg2Rad(rotation->x);
		rotation->y = Math::Deg2Rad(rotation->y);
		rotation->z = Math::Deg2Rad(rotation->z);

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

#pragma endregion

#pragma region Camera Component

	static void CameraComponent_GetPrimary(UUID entityUUID, bool* outPrimary)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outPrimary = entity.GetComponent<CameraComponent>().Primary;
	}

	static void CameraComponent_SetPrimary(UUID entityUUID, bool primary)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CameraComponent>().Primary = primary;
	}
	
	static void CameraComponent_GetFixedAspectRatio(UUID entityUUID, bool* outFixedAspectRatio)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outFixedAspectRatio = entity.GetComponent<CameraComponent>().FixedAspectRatio;
	}
	
	static void CameraComponent_SetFixedAspectRatio(UUID entityUUID, bool fixedAspectRatio)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CameraComponent>().FixedAspectRatio = fixedAspectRatio;
	}

#pragma endregion

#pragma region Sprite Renderer Component

	static void SpriteRendererComponent_GetColor(UUID entityUUID, Math::vec4* outColor)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outColor = entity.GetComponent<SpriteRendererComponent>().SpriteColor;
	}

	static void SpriteRendererComponent_SetColor(UUID entityUUID, Math::vec4* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteRendererComponent>().SpriteColor = *color;
	}

	static void SpriteRendererComponent_GetScale(UUID entityUUID, float outScale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outScale = entity.GetComponent<SpriteRendererComponent>().Scale;
	}

	static void SpriteRendererComponent_GetTexture(UUID entityUUID, MonoString* outTexturePathString)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		const std::string& texturePath = entity.GetComponent<SpriteRendererComponent>().Texture->GetPath();

		outTexturePathString = mono_string_new_wrapper(texturePath.c_str());
	}

	static void SpriteRendererComponent_SetTexture(UUID entityUUID, MonoString* texturePathString)
	{
		char* texturePathCStr = mono_string_to_utf8(texturePathString);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteRendererComponent>().Texture = Texture2D::Create(std::string(texturePathCStr));

		mono_free(texturePathCStr);
	}

	static void SpriteRendererComponent_SetScale(UUID entityUUID, float scale)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<SpriteRendererComponent>().Scale = scale;
	}

#pragma endregion

#pragma region Circle Renderer Component

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

#pragma endregion

#pragma region Rigidbody2D Component

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

#pragma endregion

#pragma region Physics2D

	// Derived class of Box2D RayCastCallback
	class RayCastCallback : public b2RayCastCallback
	{
	public:
		RayCastCallback() : fixture(nullptr) { }

		float ReportFixture(b2Fixture* fixture_, const b2Vec2& point_, const b2Vec2& normal_, float fraction_) override
		{
			fixture = fixture_;
			point = point_;
			normal = normal_;
			fraction = fraction_;
			return fraction;
		}

		b2Fixture* fixture; // This is the fixture that was hit by the raycast
		b2Vec2 point;
		b2Vec2 normal;
		float fraction;
	};

	struct RayCastHit2D
	{
		Math::vec2 Point;
		Math::vec2 Normal;
		MonoString* Tag;
		bool Hit;

		RayCastHit2D(const RayCastCallback& raycastInfo, Scene* contextScene)
		{
			Hit = raycastInfo.fixture != nullptr;

			if (Hit)
			{
				Point = Math::vec2(raycastInfo.point.x, raycastInfo.point.y);
				Normal = Math::vec2(raycastInfo.normal.x, raycastInfo.normal.y);
				Tag = mono_string_new_wrapper(contextScene->GetEntityWithUUID(reinterpret_cast<PhysicsBodyData*>(raycastInfo.fixture->GetUserData().pointer)->EntityUUID).GetName().c_str());
			}
			else
			{
				Point = Math::vec2();
				Normal = Math::vec2();
				Tag = mono_string_new_wrapper("");
			}
		}
	};

	static uint64_t Physics2D_Raycast(Math::vec2* start, Math::vec2* end, RayCastHit2D* outResult, bool drawDebugLine)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		
		// Create an instance of the callback and initialize it
		RayCastCallback raycastCallback;
		contextScene->GetPhysicsWorld()->RayCast(&raycastCallback, { start->x, start->y }, { end->x, end->y });

		*outResult = RayCastHit2D(raycastCallback, contextScene);

		// Render Raycast Hits
		if (drawDebugLine && outResult->Hit)
		{
			Renderer2D::DrawLine({ start->x, start->y, 0.0f }, { end->x, end->y, 0.0f }, s_RaycastDebugLineColor);
			Renderer2D::Flush();
		}

		if (outResult->Hit)
			return reinterpret_cast<PhysicsBodyData*>(raycastCallback.fixture->GetUserData().pointer)->EntityUUID;
		else
			return 0; // Invalid entity
	}

#pragma endregion

#pragma region Box Collider2D Component

	static void BoxCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<BoxCollider2DComponent>().Offset;
	}

	static void BoxCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().Offset = *offset;
	}
	
	static void BoxCollider2DComponent_GetSize(UUID entityUUID, Math::vec2* outSize)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outSize = entity.GetComponent<BoxCollider2DComponent>().Size;
	}
	
	static void BoxCollider2DComponent_SetSize(UUID entityUUID, Math::vec2* size)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<BoxCollider2DComponent>().Size = *size;
	}

	static void BoxCollider2DComponent_GetDensity(UUID entityUUID, float outDensity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outDensity = entity.GetComponent<BoxCollider2DComponent>().Density;
	}

	static void BoxCollider2DComponent_SetDensity(UUID entityUUID, float density)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetDensity(density);
		// Since we changed the density we must recalculate the mass data according to box2d
		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->GetBody()->ResetMassData();
	}

	static void BoxCollider2DComponent_GetFriction(UUID entityUUID, float outFriction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outFriction = entity.GetComponent<BoxCollider2DComponent>().Friction;
	}

	static void BoxCollider2DComponent_SetFriction(UUID entityUUID, float friction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetFriction(friction);
	}

	static void BoxCollider2DComponent_GetRestitution(UUID entityUUID, float outRestitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outRestitution = entity.GetComponent<BoxCollider2DComponent>().Restitution;
	}

	static void BoxCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetRestitution(restitution);
	}

	static void BoxCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float outRestitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outRestitutionThreshold = entity.GetComponent<BoxCollider2DComponent>().RestitutionThreshold;
	}

	static void BoxCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture)->SetRestitutionThreshold(restitutionThreshold);
	}

#pragma endregion

#pragma region Circle Collider2D Component

	static void CircleCollider2DComponent_GetOffset(UUID entityUUID, Math::vec2* outOffset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outOffset = entity.GetComponent<CircleCollider2DComponent>().Offset;
	}

	static void CircleCollider2DComponent_SetOffset(UUID entityUUID, Math::vec2* offset)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().Offset = *offset;
	}

	static void CircleCollider2DComponent_GetRadius(UUID entityUUID, float* outRadius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		*outRadius = entity.GetComponent<CircleCollider2DComponent>().Radius;
	}

	static void CircleCollider2DComponent_SetRadius(UUID entityUUID, float radius)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		entity.GetComponent<CircleCollider2DComponent>().Radius = radius;
	}

	static void CircleCollider2DComponent_GetDensity(UUID entityUUID, float outDensity)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outDensity = entity.GetComponent<CircleCollider2DComponent>().Density;
	}

	static void CircleCollider2DComponent_SetDensity(UUID entityUUID, float density)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetDensity(density);
		// Since we changed the density we must recalculate the mass data according to box2d
		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->GetBody()->ResetMassData();
	}

	static void CircleCollider2DComponent_GetFriction(UUID entityUUID, float outFriction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outFriction = entity.GetComponent<CircleCollider2DComponent>().Friction;
	}

	static void CircleCollider2DComponent_SetFriction(UUID entityUUID, float friction)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetFriction(friction);
	}

	static void CircleCollider2DComponent_GetRestitution(UUID entityUUID, float outRestitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outRestitution = entity.GetComponent<CircleCollider2DComponent>().Restitution;
	}

	static void CircleCollider2DComponent_SetRestitution(UUID entityUUID, float restitution)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetRestitution(restitution);
	}

	static void CircleCollider2DComponent_GetRestitutionThreshold(UUID entityUUID, float outRestitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		outRestitutionThreshold = entity.GetComponent<CircleCollider2DComponent>().RestitutionThreshold;
	}

	static void CircleCollider2DComponent_SetRestitutionThreshold(UUID entityUUID, float restitutionThreshold)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);
		SP_CORE_ASSERT(entity, "Invalid Entity UUID!");

		((b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture)->SetRestitution(restitutionThreshold);
	}

#pragma endregion

#pragma region Random

	static int RandomDevice_RangedInt32(int min, int max)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		std::random_device randomDevice;
		std::mt19937 engine(randomDevice());
		std::uniform_int_distribution<int> uniformDistribution(min, max);

		return uniformDistribution(engine);
	}

	static float RandomDevice_RangedFloat(float min, float max)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		static bool seedGenerated = false;

		if (!seedGenerated)
		{
			srand((uint32_t)time(0));
			seedGenerated = true;
		}

		auto createRandomFloat = [max]() { return (float)rand() / (float)RAND_MAX * (max); };

		float randomValue = createRandomFloat();

		while (randomValue < min)
			randomValue = createRandomFloat();

		return randomValue;
	}

#pragma endregion

#pragma region Algebra

	static Math::vec3* Algebra_CrossProductVec3(Math::vec3* left, Math::vec3* right)
	{
		Math::vec3 result = Math::Cross(*left, *right);
		return &result;
	}

	static void Renderer_SetClearColor(Math::vec3* color)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");

		RenderCommand::SetClearColor(*color);
	}

#pragma endregion

#pragma region Input

	static bool Input_IsKeyDown(KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}
	
	static bool Input_IsKeyUp(KeyCode key)
	{
		return Input::IsKeyReleased(key);
	}

	// TODO Input_Mouse functions

#pragma endregion

#pragma region Debug

	static void Debug_Log(MonoString* message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_TRACE("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Info(MonoString * message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_INFO("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Warn(MonoString * message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_WARN("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Error(MonoString * message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_ERROR("{}", managedString);

		mono_free(managedString);
	}
	
	static void Debug_Critical(MonoString * message)
	{
		char* managedString = mono_string_to_utf8(message);

		Scene* contextScene = ScriptEngine::GetContextScene();
		SP_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
		SP_CRITICAL("{}", managedString);

		mono_free(managedString);
	}

#pragma endregion

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
		s_EntityHasComponentFuncs.clear();
		RegisterComponent(AllComponents{});
	}

	void ScriptRegistry::RegisterMethods()
	{

#pragma region Game

		SP_ADD_INTERNAL_CALL(Application_Shutdown);

#pragma endregion

#pragma region Entity

		SP_ADD_INTERNAL_CALL(Entity_HasComponent);
		SP_ADD_INTERNAL_CALL(Entity_AddCamera);
		SP_ADD_INTERNAL_CALL(Entity_RemoveCamera);
		SP_ADD_INTERNAL_CALL(Entity_AddSpriteRenderer);
		SP_ADD_INTERNAL_CALL(Entity_RemoveSpriteRenderer);
		SP_ADD_INTERNAL_CALL(Entity_AddCircleRenderer);
		SP_ADD_INTERNAL_CALL(Entity_RemoveCircleRenderer);
		SP_ADD_INTERNAL_CALL(Entity_AddRigidBody2D);
		SP_ADD_INTERNAL_CALL(Entity_RemoveRigidBody2D);
		SP_ADD_INTERNAL_CALL(Entity_AddBoxCollider2D);
		SP_ADD_INTERNAL_CALL(Entity_RemoveBoxCollider2D);
		SP_ADD_INTERNAL_CALL(Entity_AddCircleCollider2D);
		SP_ADD_INTERNAL_CALL(Entity_RemoveCircleCollider2D);
		SP_ADD_INTERNAL_CALL(Entity_CreateWithName);
		SP_ADD_INTERNAL_CALL(Entity_GetName);
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

#pragma region Camera Component

		SP_ADD_INTERNAL_CALL(CameraComponent_GetPrimary);
		SP_ADD_INTERNAL_CALL(CameraComponent_SetPrimary);
		SP_ADD_INTERNAL_CALL(CameraComponent_GetFixedAspectRatio);
		SP_ADD_INTERNAL_CALL(CameraComponent_SetFixedAspectRatio);

#pragma endregion

#pragma region Sprite Renderer Component

		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTexture);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_GetScale);
		SP_ADD_INTERNAL_CALL(SpriteRendererComponent_SetScale);

#pragma endregion

#pragma region Circle Renderer Component

		SP_ADD_INTERNAL_CALL(CircleRendererComponent_GetColor);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_SetColor);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_GetThickness);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_SetThickness);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_GetFade);
		SP_ADD_INTERNAL_CALL(CircleRendererComponent_SetFade);

#pragma endregion

#pragma region Physics2D

		SP_ADD_INTERNAL_CALL(Physics2D_Raycast);

#pragma endregion

#pragma region RigidBody2D Component

		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyForce);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyForceToCenter);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulse);
		SP_ADD_INTERNAL_CALL(RigidBody2DComponent_ApplyLinearImpulseToCenter);

#pragma endregion

#pragma region Box Collider2D Component

		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetOffset);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetOffset);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetSize);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetSize);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetDensity);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetDensity);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetFriction);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetFriction);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitution);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitution);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetRestitutionThreshold);
		SP_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetRestitutionThreshold);

#pragma endregion

#pragma region Circle Collider2D Component

		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetOffset);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetOffset);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRadius);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRadius);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetDensity);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetDensity);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetFriction);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetFriction);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRestitution);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRestitution);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRestitutionThreshold);
		SP_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRestitutionThreshold);

#pragma endregion

#pragma region RandomDevice

		SP_ADD_INTERNAL_CALL(RandomDevice_RangedInt32);
		SP_ADD_INTERNAL_CALL(RandomDevice_RangedFloat);

#pragma endregion

#pragma region Algebra

		SP_ADD_INTERNAL_CALL(Algebra_CrossProductVec3);

#pragma endregion

#pragma region Renderer

		SP_ADD_INTERNAL_CALL(Renderer_SetClearColor);

#pragma endregion

#pragma region Input

		SP_ADD_INTERNAL_CALL(Input_IsKeyDown);
		SP_ADD_INTERNAL_CALL(Input_IsKeyUp);

#pragma endregion

#pragma region Debug

		SP_ADD_INTERNAL_CALL(Debug_Log);
		SP_ADD_INTERNAL_CALL(Debug_Info);
		SP_ADD_INTERNAL_CALL(Debug_Warn);
		SP_ADD_INTERNAL_CALL(Debug_Error);
		SP_ADD_INTERNAL_CALL(Debug_Critical);

#pragma endregion

	}

}
