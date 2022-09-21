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

namespace Sparky {

#define SP_ADD_INTERNAL_CALL(icall) mono_add_internal_call("Sparky.InternalCalls::" #icall, icall)

	static void NativeLog(MonoString* text, int parameter)
	{
		char* cStr = mono_string_to_utf8(text);
		std::string string(cStr);

		mono_free(cStr);
		SP_CORE_TRACE("text = {}, parameter = {}", string, parameter);
	}

	static void NativeLog_Vector(Math::vec3* parameter, Math::vec3* outParameter)
	{
		SP_CORE_WARN("Vector3 from C# {}", *parameter);

		*outParameter = Math::Cross(*parameter, Math::vec3(parameter->x, parameter->y, -parameter->z));
	}

	static void Entity_GetTranslation(UUID entityUUID, Math::vec3* outTranslation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;
	}

	static void Entity_SetTranslation(UUID entityUUID, Math::vec3* translation)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();
		
		Entity entity = contextScene->GetEntityWithUUID(entityUUID);

		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	static bool Input_IsKeyDown(KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}

	void ScriptRegistry::RegisterMethods()
	{
		SP_ADD_INTERNAL_CALL(NativeLog);
		SP_ADD_INTERNAL_CALL(NativeLog_Vector);

		SP_ADD_INTERNAL_CALL(Entity_GetTranslation);
		SP_ADD_INTERNAL_CALL(Entity_SetTranslation);

		SP_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}

}