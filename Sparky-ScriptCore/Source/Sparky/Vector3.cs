namespace Sparky {

	public struct Vector3
	{
		public float X, Y, Z;

		public static Vector3 Zero => new Vector3(0.0f);
		public static Vector3 Forward => new Vector3(0.0f, 0.0f, -1.0f); // In OpenGL forward is negative on the Z axis
		public static Vector3 Back => new Vector3(0.0f, 0.0f, 1.0f);
		public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
		public static Vector3 Down => new Vector3(0.0f, -1.0f, 0.0f);
		public static Vector3 Left => new Vector3(-1.0f, 0.0f, 0.0f);
		public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);

		public override string ToString()
		{
			return $"Vector3: {X}, {Y}, {Z}";
		}

		public Vector3(float scalar)
		{
			X = scalar;
			Y = scalar;
			Z = scalar;
		}

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public Vector3(Vector2 xy, float z)
		{
			X = xy.X;
			Y = xy.Y;
			Z = z;
		}

		public Vector2 XY
		{
			get => new Vector2(X, Y);
			set
			{
				X = value.X;
				Y = value.Y;
			}
		}

		public static Vector3 operator +(Vector3 vector, Vector3 other)
		{
			return new Vector3(vector.X + other.X, vector.Y + other.Y, vector.Z + other.Z);
		}

		public static Vector3 operator *(Vector3 vector, Vector3 other)
		{
			return new Vector3(vector.X * other.X, vector.Y * other.Y, vector.Z * other.Z);
		}

		public static Vector3 operator *(Vector3 vector, float scalar)
		{
			return new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
		}

		public static bool operator ==(Vector3 vector, Vector3 other)
		{
			return vector.X == other.X && vector.Y == other.Y && vector.Z == other.Z;
		}

		public static bool operator !=(Vector3 vector, Vector3 other)
		{
			return !(vector == other);
		}
	}

}
