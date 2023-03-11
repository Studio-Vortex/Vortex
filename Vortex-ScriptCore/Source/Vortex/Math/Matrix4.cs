using System.Runtime.InteropServices;

namespace Vortex {

	[StructLayout(LayoutKind.Explicit)]
	public struct Matrix4
	{
		[FieldOffset( 0)] public float M00;
		[FieldOffset( 4)] public float M10;
		[FieldOffset( 8)] public float M20;
		[FieldOffset(12)] public float M30;

		[FieldOffset(16)] public float M01;
		[FieldOffset(20)] public float M11;
		[FieldOffset(24)] public float M21;
		[FieldOffset(28)] public float M31;

		[FieldOffset(32)] public float M02;
		[FieldOffset(36)] public float M12;
		[FieldOffset(40)] public float M22;
		[FieldOffset(44)] public float M32;

		[FieldOffset(48)] public float M03;
		[FieldOffset(52)] public float M13;
		[FieldOffset(56)] public float M23;
		[FieldOffset(60)] public float M33;

		public Vector4 Row0 => new Vector4(M00, M10, M20, M30);
		public Vector4 Row1 => new Vector4(M01, M11, M21, M31);
		public Vector4 Row2 => new Vector4(M02, M12, M22, M32);
		public Vector4 Row3 => new Vector4(M03, M13, M23, M33);

		public static Matrix4 Identity => new Matrix4(1.0f);

		public Matrix4(float v)
		{
			M00 = v; M10 = 0; M20 = 0; M30 = 0;
			M01 = 0; M11 = v; M21 = 0; M31 = 0;
			M02 = 0; M12 = 0; M22 = v; M32 = 0;
			M03 = 0; M13 = 0; M23 = 0; M33 = v;
		}

		public Matrix4(Vector4 row0, Vector4 row1, Vector4 row2, Vector4 row3)
		{
			M00 = row0.X; M10 = row0.Y; M20 = row0.Z; M30 = row0.W;
			M01 = row0.X; M11 = row0.Y; M21 = row0.Z; M31 = row0.W;
			M02 = row0.X; M12 = row0.Y; M22 = row0.Z; M32 = row0.W;
			M03 = row0.X; M13 = row0.Y; M23 = row0.Z; M33 = row0.W;
		}

		public Vector3 Translation
		{
			get => new Vector3(M03, M13, M23);
			set { M03 = value.X; M13 = value.Y; M23 = value.Z; }
		}

		public static Matrix4 Translate(Vector3 translation)
		{
			return new Matrix4(1.0f)
			{
				M03 = translation.X,
				M13 = translation.Y,
				M23 = translation.Z,
			};
		}

		public static Matrix4 Rotate(float angle, Vector3 axis)
		{
			InternalCalls.Matrix4_Rotate(angle, ref axis, out Matrix4 result);
			return result;
		}

		public Vector3 Scale
		{
			get => new Vector3(M00, M11, M22);
			set { M00 = value.X; M11 = value.Y; M22 = value.Z; }
		}

		public static Matrix4 ApplyScale(float scale)
		{
			return new Matrix4(1.0f)
			{
				M00 = scale,
				M11 = scale,
				M22 = scale,
			};
		}

		public static Matrix4 ApplyScale(Vector3 scale)
		{
			return new Matrix4(1.0f)
			{
				M00 = scale.X,
				M11 = scale.Y,
				M22 = scale.Z,
			};
		}

		public static Matrix4 LookAt(Vector3 eye, Vector3 point, Vector3 up)
		{
			InternalCalls.Matrix4_LookAt(ref eye, ref point, ref up, out Matrix4 result);
			return result;
		}

		public static Matrix4 operator *(Matrix4 matrix, Matrix4 other)
		{
			InternalCalls.Matrix4_Multiply(ref matrix, ref other, out Matrix4 result);
			return result;
		}
	}

}
