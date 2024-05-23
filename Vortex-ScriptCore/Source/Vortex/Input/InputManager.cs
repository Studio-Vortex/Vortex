namespace Vortex {

	public static class InputManager
	{
		public static KeyCode GetKeybind(string name) => InternalCalls.InputManager_GetKeybind(name);
		public static MouseButton GetMousebind(string name) => InternalCalls.InputManager_GetMousebind(name);

		public static bool HasKeybind(string name) => InternalCalls.InputManager_HasKeybind(name);
		public static bool HasMousebind(string name) => InternalCalls.InputManager_HasMousebind(name);
	}

}
