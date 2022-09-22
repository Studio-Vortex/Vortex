namespace Sparky {

	public struct Vector2
	{
		public float X, Y;

		public static Vector2 Zero => new Vector2(0.0f);

		public Vector2(float scalar)
		{
			X = scalar;
			Y = scalar;
		}

		public Vector2(float x, float y)
		{
			X = x;
			Y = y;
		}

		public static Vector2 operator +(Vector2 vector, Vector2 other)
		{
			return new Vector2(vector.X + other.X, vector.Y + other.Y);
		}

		public static Vector2 operator *(Vector2 vector, float scalar)
		{
			return new Vector2(vector.X * scalar, vector.Y * scalar);
		}
	}

}
