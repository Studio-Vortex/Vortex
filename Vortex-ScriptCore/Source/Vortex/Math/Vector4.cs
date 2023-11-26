using System;
using System.Runtime.InteropServices;

namespace Vortex {

	[StructLayout(LayoutKind.Sequential)]
	public struct Vector4 : IEquatable<Vector4>
	{
		public float X, Y, Z, W;

		public static Vector4 Zero => new Vector4(0.0f);
		public static Vector4 One => new Vector4(1.0f);
		public static Vector4 Forward => new Vector4(0.0f, 0.0f, -1.0f, 0.0f); // In OpenGL forward is negative on the Z axis
		public static Vector4 Back => new Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		public static Vector4 Up => new Vector4(0.0f, 1.0f, 0.0f, 0.0f);
		public static Vector4 Down => new Vector4(0.0f, -1.0f, 0.0f, 0.0f);
		public static Vector4 Left => new Vector4(-1.0f, 0.0f, 0.0f, 0.0f);
		public static Vector4 Right => new Vector4(1.0f, 0.0f, 0.0f, 0.0f);

		public override string ToString()
		{
			return $"Vector4: {X}, {Y}, {Z}, {W}";
		}

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

		public Vector4(float xyz, float w)
		{
			X = xyz;
			Y = xyz;
			Z = xyz;
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

		public static Vector4 operator +(Vector4 vector, Vector4 other) => new Vector4(vector.X + other.X, vector.Y + other.Y, vector.Z + other.Z, vector.W + other.W);
		public static Vector4 operator +(Vector4 vector, float scalar) => new Vector4(vector.X + scalar, vector.Y + scalar, vector.Z + scalar, vector.W + scalar);
		public static Vector4 operator +(float scalar, Vector4 vector) => new Vector4(scalar + vector.X, scalar + vector.Y, scalar + vector.Z, scalar + vector.W);
		public static Vector4 operator -(Vector4 vector, Vector4 other) => new Vector4(vector.X - other.X, vector.Y - other.Y, vector.Z - other.Z, vector.W - other.W);
		public static Vector4 operator -(Vector4 vector, float scalar) => new Vector4(vector.X - scalar, vector.Y - scalar, vector.Z - scalar, vector.W - scalar);
		public static Vector4 operator -(float scalar, Vector4 vector) => new Vector4(scalar - vector.X, scalar - vector.Y, scalar - vector.Z, scalar - vector.W);
		public static Vector4 operator -(Vector4 vector) => new Vector4(-vector.X, -vector.Y, -vector.Z, -vector.W);
		public static Vector4 operator *(Vector4 vector, Vector4 other) => new Vector4(vector.X * other.X, vector.Y * other.Y, vector.Z * other.Z, vector.W * other.W);
		public static Vector4 operator *(Vector4 vector, float scalar) => new Vector4(vector.X * scalar, vector.Y * scalar, vector.Z * scalar, vector.W * scalar);
		public static Vector4 operator *(float scalar, Vector4 vector) => new Vector4(scalar * vector.X, scalar * vector.Y, scalar * vector.Z, scalar * vector.W);
		public static Vector4 operator /(Vector4 vector, Vector4 other) => new Vector4(vector.X / other.X, vector.Y / other.Y, vector.Z / other.Z, vector.W / other.W);
		public static Vector4 operator /(Vector4 vector, float scalar) => new Vector4(vector.X / scalar, vector.Y / scalar, vector.Z / scalar, vector.W / scalar);
		public static Vector4 operator /(float scalar, Vector4 vector) => new Vector4(scalar / vector.X, scalar / vector.Y, scalar / vector.Z, scalar / vector.W);

		public static bool operator <(Vector4 vector, Vector4 other) => vector.X < other.X && vector.Y < other.Y && vector.Z < other.Z && vector.W < other.W;
		public static bool operator >(Vector4 vector, Vector4 other) => !(vector < other);
		public static bool operator <=(Vector4 vector, Vector4 other) => vector.X <= other.X && vector.Y <= other.Y && vector.Z <= other.Z && vector.W <= other.W;
		public static bool operator >=(Vector4 vector, Vector4 other) => !(vector <= other);

		public static bool operator <(Vector4 vector, float scalar) => vector.X < scalar && vector.Y < scalar && vector.Z < scalar && vector.W < scalar;
		public static bool operator >(Vector4 vector, float scalar) => !(vector < scalar);
		public static bool operator <=(Vector4 vector, float scalar) => vector.X <= scalar && vector.Y <= scalar && vector.Z <= scalar && vector.W <= scalar;
		public static bool operator >=(Vector4 vector, float scalar) => !(vector <= scalar);

		public static bool operator ==(Vector4 left, Vector4 right) => left.Equals(right);
		public static bool operator !=(Vector4 left, Vector4 right) => !(left == right);

		public static implicit operator Color4(Vector4 vector) => new Color4(vector.X, vector.Y, vector.Z, vector.W);

		public override bool Equals(object obj) => obj is Vector4 other && Equals(other);
		public bool Equals(Vector4 right) => X == right.X && Y == right.Y && Z == right.Z && W == right.W;

		public override int GetHashCode() => (X, Y, Z, W).GetHashCode();

		public float Length() => Mathf.Sqrt(X * X + Y * Y + Z * Z + W * W);

		public static Vector4 Lerp(Vector4 p1, Vector4 p2, float maxDistanceDelta)
		{
			if (maxDistanceDelta < 0.0f)
				return p1;

			if (maxDistanceDelta > 1.0f)
				return p2;

			return p1 + ((p2 - p1) * maxDistanceDelta);
		}

		public Vector4 Clamp(Vector4 value, Vector4 min, Vector4 max)
		{
			return new Vector4(
				Mathf.Clamp(value.X, min.X, max.X),
				Mathf.Clamp(value.Y, min.Y, max.Y),
				Mathf.Clamp(value.Z, min.Z, max.Z),
				Mathf.Clamp(value.W, min.W, max.W)
			);
		}
	}

}
