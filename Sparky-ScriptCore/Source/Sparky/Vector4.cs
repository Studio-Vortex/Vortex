namespace Sparky {

	public struct Vector4
	{
		public float X, Y, Z, W;

		public static Vector4 Zero => new Vector4(0.0f);
		public static Vector4 Forward => new Vector4(0.0f, 0.0f, -1.0f, 0.0f); // In OpenGL forward is negative on the Z axis
		public static Vector4 Back => new Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		public static Vector4 Up => new Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		public static Vector4 Down => new Vector4(0.0f, -1.0f, 0.0f, 0.0f);
		public static Vector4 Left => new Vector4(-1.0f, 0.0f, 0.0f, 0.0f);
		public static Vector4 Right => new Vector4(1.0f, 0.0f, 0.0f, 0.0f);

		public Vector4(float scalar)
		{
			X = scalar;
			Y = scalar;
			Z = scalar;
			W = scalar;
		}

		public Vector4(float x, float y, float z, float w)
		{
			X = x;
			Y = y;
			Z = z;
			W = w;
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

		public Vector3 XYZ
		{
			get => new Vector3(X, Y, Z);
			set
			{
				X = value.X;
				Y = value.Y;
				Z = value.Z;
			}
		}

		public static Vector4 operator +(Vector4 vector, Vector4 other)
		{
			return new Vector4(vector.X + other.X, vector.Y + other.Y, vector.Z + other.Z, vector.W + other.W);
		}

		public static Vector4 operator *(Vector4 vector, Vector4 other)
		{
			return new Vector4(vector.X * other.X, vector.Y * other.Y, vector.Z * other.Z, vector.W * other.W);
		}

		public static Vector4 operator *(Vector4 vector, float scalar)
		{
			return new Vector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
		}
	}

}
