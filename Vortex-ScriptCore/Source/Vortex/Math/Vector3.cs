using System;
using System.Runtime.InteropServices;

namespace Vortex {

	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3 : IEquatable<Vector3>
	{
		public float X, Y, Z;

		public static Vector3 Zero => new Vector3(0.0f);
		public static Vector3 One => new Vector3(1.0f);
		public static Vector3 Forward => new Vector3(0.0f, 0.0f, -1.0f); // In OpenGL forward is negative on the Z axis
		public static Vector3 Back => new Vector3(0.0f, 0.0f, 1.0f);
		public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
		public static Vector3 Down => new Vector3(0.0f, -1.0f, 0.0f);
		public static Vector3 Left => new Vector3(-1.0f, 0.0f, 0.0f);
		public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);

		public Vector3(float scalar) => X = Y = Z = scalar;
		public Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }
		public Vector3(Vector2 xy, float z) { X = xy.X; Y = xy.Y; Z = z; }
		public Vector3(float x, Vector2 yz) { X = x; Y = yz.X; Z = yz.Y; }

		public Vector2 XY
		{
			get => new Vector2(X, Y);
			set { X = value.X; Y = value.Y; }
		}

		public Vector2 YZ
		{
			get => new Vector2(Y, Z);
			set { Y = value.X; Z = value.Y; }
		}

		public Vector2 XZ
		{
			get => new Vector2(X, Z);
			set { X = value.X; Z = value.Y; }
		}

		public const float kEpsilonNormalSqrt = 1e-15F;

		public float sqrMagnitude { get { return X * X + Y * Y + Z * Z; } }

		public static Vector3 operator +(Vector3 vector, Vector3 other) => new Vector3(vector.X + other.X, vector.Y + other.Y, vector.Z + other.Z);
		public static Vector3 operator +(Vector3 vector, float scalar) => new Vector3(vector.X + scalar, vector.Y + scalar, vector.Z + scalar);
		public static Vector3 operator +(float scalar, Vector3 vector) => new Vector3(vector.X + scalar, vector.Y + scalar, vector.Z + scalar);
		public static Vector3 operator -(Vector3 vector, Vector3 other) => new Vector3(vector.X - other.X, vector.Y - other.Y, vector.Z - other.Z);
		public static Vector3 operator -(Vector3 vector, float scalar) => new Vector3(vector.X - scalar, vector.Y - scalar, vector.Z - scalar);
		public static Vector3 operator -(float scalar, Vector3 vector) => new Vector3(vector.X - scalar, vector.Y - scalar, vector.Z - scalar);
		public static Vector3 operator -(Vector3 vector) => new Vector3(-vector.X, -vector.Y, -vector.Z);
		public static Vector3 operator *(Vector3 vector, Vector3 other) => new Vector3(vector.X * other.X, vector.Y * other.Y, vector.Z * other.Z);
		public static Vector3 operator *(Vector3 vector, float scalar) => new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
		public static Vector3 operator *(float scalar, Vector3 vector) => new Vector3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
		public static Vector3 operator /(Vector3 vector, Vector3 other) => new Vector3(vector.X / other.X, vector.Y / other.Y, vector.Z / other.Z);
		public static Vector3 operator /(Vector3 vector, float scalar) => new Vector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
		public static Vector3 operator /(float scalar, Vector3 vector) => new Vector3(vector.X / scalar, vector.Y / scalar, vector.Z / scalar);
		
		public static bool operator <(Vector3 vector, Vector3 other) => vector.X < other.X && vector.Y < other.Y && vector.Z < other.Z;
		public static bool operator >(Vector3 vector, Vector3 other) => !(vector < other);
		public static bool operator <=(Vector3 vector, Vector3 other) => vector.X <= other.X && vector.Y <= other.Y && vector.Z <= other.Z;
		public static bool operator >=(Vector3 vector, Vector3 other) => !(vector <= other);

		public static bool operator <(Vector3 vector, float scalar) => vector.X < scalar && vector.Y < scalar && vector.Z < scalar;
		public static bool operator >(Vector3 vector, float scalar) => !(vector < scalar);
		public static bool operator <=(Vector3 vector, float scalar) => vector.X <= scalar && vector.Y <= scalar && vector.Z <= scalar;
		public static bool operator >=(Vector3 vector, float scalar) => !(vector <= scalar);

		public static bool operator ==(Vector3 left, Vector3 right) => left.Equals(right);
		public static bool operator !=(Vector3 left, Vector3 right) => !(left == right);

		public override bool Equals(object obj) => obj is Vector3 other && Equals(other);
		public bool Equals(Vector3 right) => X == right.X && Y == right.Y && Z == right.Z;

		public override int GetHashCode() => (X, Y, Z).GetHashCode();

		public float Length() => Mathf.Sqrt(X * X + Y * Y + Z * Z);

		public Vector3 Normalized()
		{
			float length = Length();

			if (length > 0.0f)
				return new Vector3(X / length, Y / length, Z / length);
			else
				return new Vector3(X, Y, Z);
		}

		public void Normalize()
		{
			float length = Length();
			X /= length;
			Y /= length;
			Z /= length;
		}

		public static float Distance(Vector3 lhs, Vector3 rhs)
		{
			return Mathf.Sqrt(Mathf.Pow(rhs.X - lhs.X, 2) +
							  Mathf.Pow(rhs.Y - lhs.Y, 2) +
							  Mathf.Pow(rhs.Z - lhs.Z, 2));
		}

		public static Vector3 Lerp(Vector3 p1, Vector3 p2, float maxDistanceDelta)
		{
			if (maxDistanceDelta < 0.0f)
				return p1;

			if (maxDistanceDelta > 1.0f)
				return p2;

			return p1 + ((p2 - p1) * maxDistanceDelta);
		}

		public static Vector3 Cross(Vector3 left, Vector3 right)
		{
			float a1 = left.X;
			float a2 = left.Y;
			float a3 = left.Z;

			float b1 = left.X;
			float b2 = left.Y;
			float b3 = left.Z;

			float x = a2 * b3 - a3 * b2;
			float y = a3 * b1 - a1 * b3;
			float z = a1 * b2 - a2 * b1;

			return new Vector3(x, y, z);
		}

		public static float Dot(Vector3 left, Vector3 right)
		{
			return left.X * right.X + left.Y * right.Y + left.Z * right.Z;
		}

		public static Vector3 Cos(Vector3 other) => new Vector3(Mathf.Cos(other.X), Mathf.Cos(other.Y), Mathf.Cos(other.Z));
		public static Vector3 Sin(Vector3 other) => new Vector3(Mathf.Sin(other.X), Mathf.Sin(other.Y), Mathf.Sin(other.Z));

		public static float Angle(Vector3 from, Vector3 to)
		{
			// sqrt(a) * sqrt(b) = sqrt(a * b) -- valid for real numbers
			float denominator = (float)Math.Sqrt(from.sqrMagnitude * to.sqrMagnitude);
			if (denominator < kEpsilonNormalSqrt)
				return 0F;

			float dot = Mathf.Clamp(Dot(from, to) / denominator, -1F, 1F);
			return Mathf.Rad2Deg(Mathf.Acos(dot));
		}

		public override string ToString() { return $"Vector3: {X}, {Y}, {Z}"; }

		public static implicit operator Color3(Vector3 vector) => new Color3(vector.X, vector.Y, vector.Z);
	}

}
