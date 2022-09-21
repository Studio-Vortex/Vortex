using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sparky
{

	public static class Input
	{
		public static bool IsKeyDown(KeyCode key)
		{
			return InternalCalls.Input_IsKeyDown(key);
		}
	}

}
