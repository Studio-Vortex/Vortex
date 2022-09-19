#include "sppch.h"
#include "ScriptEngine.h"

#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>

namespace Sparky {

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
	}

	void ScriptEngine::Shutdown()
	{
	}

}