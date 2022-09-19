#pragma once

namespace Sparky {

	class ScriptEngine
	{
	public:
		ScriptEngine() = delete;
		ScriptEngine(const ScriptEngine&) = delete;

		static void Init();
		static void Shutdown();

	private:
		static void InitMono();
		static void ShutdownMono();
	};

}
