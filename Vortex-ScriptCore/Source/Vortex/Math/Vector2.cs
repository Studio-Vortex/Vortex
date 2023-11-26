namespace Vortex {

	public struct Vector2
	{
		public float X, Y;

		public static Vector2 Zero => new Vector2(0.0f);
		public static Vector2 One => new Vector2(1.0f);
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

		public static implicit operator Color2(Vector2 vector) => new Color2(vector.X, vector.Y);

		public static Vector2 Clamp(Vector2 value, Vector2 min, Vector2 max)
		{
			return new Vector2(
				Mathf.Clamp(value.X, min.X, max.X),
				Mathf.Clamp(value.Y, min.Y, max.Y)
			);
		}

		public static Vector2 Lerp(Vector2 p1, Vector2 p2, float maxDistanceDelta)
		{
			if (maxDistanceDelta < 0.0f)
				return p1;

			if (maxDistanceDelta > 1.0f)
				return p2;

			return p1 + ((p2 - p1) * maxDistanceDelta);
		}
	}

}
