﻿namespace Sparky {

	public static class Physics2D
	{
		public static bool Raycast(Vector2 start, Vector2 end)
		{
			return InternalCalls.Physics2D_Raycast(ref start, ref end);
		}
	}

}