namespace Sparky {

	public struct Vector2
	{
		public float X, Y;

		public static Vector2 Zero => new Vector2(0.0f);
		public static Vector2 Up => new Vector2(0.0f, 1.0f);
		public static Vector2 Down => new Vector2(0.0f, -1.0f);
		public static Vector2 Left => new Vector2(-1.0f, 0.0f);
		public static Vector2 Right => new Vector2(1.0f, 0.0f);

		public override string ToString()
		{
			return $"Vector2: {X}, {Y}";
		}

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

		public static Vector2 operator -(Vector2 vector)
		{
			return new Vector2(-vector.X, -vector.Y);
		}

		public static Vector2 operator +(Vector2 vector, Vector2 other)
		{
			return new Vector2(vector.X + other.X, vector.Y + other.Y);
		}

		public static Vector2 operator -(Vector2 vector, Vector2 other)
		{
			return new Vector2(vector.X - other.X, vector.Y - other.Y);
		}

		public static Vector2 operator *(Vector2 vector, Vector2 other)
		{
			return new Vector2(vector.X * other.X, vector.Y * other.Y);
		}

		public static Vector2 operator +(Vector2 vector, float scalar)
		{
			return new Vector2(vector.X + scalar, vector.Y + scalar);
		}

		public static Vector2 operator -(Vector2 vector, float scalar)
		{
			return new Vector2(vector.X - scalar, vector.Y - scalar);
		}

		public static Vector2 operator *(Vector2 vector, float scalar)
		{
			return new Vector2(vector.X * scalar, vector.Y * scalar);
		}

		public static Vector2 operator /(Vector2 vector, float scalar)
		{
			return new Vector2(vector.X / scalar, vector.Y / scalar);
		}

		public static bool operator ==(Vector2 vector, Vector2 other)
		{
			return vector.X == other.X && vector.Y == other.Y;
		}

		public static bool operator !=(Vector2 vector, Vector2 other)
		{
			return !(vector == other);
		}
	}

}
