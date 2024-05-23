#include "vxpch.h"
#include "MouseCodes.h"

namespace Vortex {

	namespace Utils {

		const char* StringFromMouseButton(MouseButton button)
		{
			switch (button)
			{
				case MouseButton::Button0: return "Left";
				case MouseButton::Button1: return "Right";
				case MouseButton::Button2: return "Middle";
				case MouseButton::Button3: return "Button3";
				case MouseButton::Button4: return "Button4";
				case MouseButton::Button5: return "Button5";
				case MouseButton::Button6: return "Button6";
				case MouseButton::Button7: return "Button7";
			}

			return "Unknown mouse button!";
		}

		MouseButton MouseButtonFromString(const char* str)
		{
			if (str == "Left") return MouseButton::Button0;
			if (str == "Right") return MouseButton::Button1;
			if (str == "Middle") return MouseButton::Button2;
			if (str == "Button3") return MouseButton::Button3;
			if (str == "Button4") return MouseButton::Button4;
			if (str == "Button5") return MouseButton::Button5;
			if (str == "Button6") return MouseButton::Button6;
			if (str == "Button7") return MouseButton::Button7;

			return MouseButton::Left;
		}

	}

}
