namespace Vortex {

	public static class Color
	{
		public static Vector4 Red          { get => new Vector4(1.0f, 0.0f, 0.0f, 1.0f); }
		public static Vector4 Green        { get => new Vector4(0.0f, 1.0f, 0.0f, 1.0f); }
		public static Vector4 Blue         { get => new Vector4(0.0f, 0.0f, 1.0f, 1.0f); }
		public static Vector4 Purple       { get => new Vector4(0.6f, 0.1f, 0.9f, 1.0f); }
		public static Vector4 Yellow       { get => new Vector4(1.0f, 1.0f, 0.0f, 1.0f); }
		public static Vector4 Orange       { get => new Vector4(1.0f, 0.4f, 0.0f, 1.0f); }
		public static Vector4 White        { get => new Vector4(1.0f, 1.0f, 1.0f, 1.0f); }
		public static Vector4 Black        { get => new Vector4(0.0f, 0.0f, 0.0f, 1.0f); }
		public static Vector4 LightRed     { get => new Vector4(0.8f, 0.2f, 0.2f, 1.0f); }
		public static Vector4 LightGreen   { get => new Vector4(0.2f, 0.8f, 0.2f, 1.0f); }
		public static Vector4 LightBlue    { get => new Vector4(0.2f, 0.2f, 0.8f, 1.0f); }
		public static Vector4 LightYellow  { get => new Vector4(0.8f, 0.8f, 0.2f, 1.0f); }
	}

	public struct Color3
	{
		public float R, G, B;

		public Color3(float r, float g, float b)
		{
			R = r;
			G = g;
			B = b;
		}

		public static implicit operator Vector3(Color3 color) => new Vector3(color.R, color.G, color.B);
	}

	public struct Color4
	{
		public float R, G, B, A;

		public Color4(float r, float g, float b, float a)
		{
			R = r;
			G = g;
			B = b;
			A = a;
		}

		public static implicit operator Vector4(Color4 color) => new Vector4(color.R, color.G, color.B, color.A);
	}

}
