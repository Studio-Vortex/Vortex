namespace Vortex {

	public static class Color
	{
		public static Color4 Red4          => new Color4(1.0f, 0.0f, 0.0f, 1.0f);
		public static Color4 Green4        => new Color4(0.0f, 1.0f, 0.0f, 1.0f);
		public static Color4 Blue4         => new Color4(0.0f, 0.0f, 1.0f, 1.0f);
		public static Color4 Purple4       => new Color4(0.6f, 0.1f, 0.9f, 1.0f);
		public static Color4 Yellow4       => new Color4(1.0f, 1.0f, 0.0f, 1.0f);
		public static Color4 Orange4       => new Color4(1.0f, 0.4f, 0.0f, 1.0f);
		public static Color4 White4        => new Color4(1.0f, 1.0f, 1.0f, 1.0f);
		public static Color4 Black4        => new Color4(0.0f, 0.0f, 0.0f, 1.0f);
		public static Color4 LightRed4     => new Color4(0.8f, 0.2f, 0.2f, 1.0f);
		public static Color4 LightGreen4   => new Color4(0.2f, 0.8f, 0.2f, 1.0f);
		public static Color4 LightBlue4    => new Color4(0.2f, 0.2f, 0.8f, 1.0f);
		public static Color4 LightYellow4  => new Color4(0.8f, 0.8f, 0.2f, 1.0f);

		public static Color3 Red           => new Color3(1.0f, 0.0f, 0.0f);
		public static Color3 Green         => new Color3(0.0f, 1.0f, 0.0f);
		public static Color3 Blue          => new Color3(0.0f, 0.0f, 1.0f);
		public static Color3 Purple        => new Color3(0.6f, 0.1f, 0.9f);
		public static Color3 Yellow        => new Color3(1.0f, 1.0f, 0.0f);
		public static Color3 Orange        => new Color3(1.0f, 0.4f, 0.0f);
		public static Color3 White         => new Color3(1.0f, 1.0f, 1.0f);
		public static Color3 Black         => new Color3(0.0f, 0.0f, 0.0f);
		public static Color3 LightRed      => new Color3(0.8f, 0.2f, 0.2f);
		public static Color3 LightGreen    => new Color3(0.2f, 0.8f, 0.2f);
		public static Color3 LightBlue     => new Color3(0.2f, 0.2f, 0.8f);
		public static Color3 LightYellow   => new Color3(0.8f, 0.8f, 0.2f);

		public static Color3 Random => new Color3(Vortex.Random.Value, Vortex.Random.Value, Vortex.Random.Value);
	}

	public struct Color2
	{
		public float R, G;

		public Color2(float r, float g)
		{
			R = r;
			G = g;
		}

		public Color2(float scalar)
		{
			R = scalar;
			G = scalar;
		}

		public Color2 RR => new Color2(R, R);
		public Color2 GG => new Color2(G, G);

		public static implicit operator Vector2(Color2 color) => new Vector2(color.R, color.G);
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

		public Color3(float scalar)
		{
			R = scalar;
			G = scalar;
			B = scalar;
		}

		public Color2 RG
		{
			get => new Color2(R, G);
			set { R = value.R; G = value.G; }
		}

		public Color2 RB
		{
			get => new Color2(R, B);
			set { R = value.R; B = value.G; }
		}

		public Color2 BG
		{
			get => new Color2(G, B);
			set { G = value.R; B = value.G; }
		}

		public Color3 RRR => new Color3(R, R, R);
		public Color3 GGG => new Color3(G, G, G);
		public Color3 BBB => new Color3(B, B, B);

		public Color2 RR => new Color2(R, R);
		public Color2 GG => new Color2(G, G);
		public Color2 BB => new Color2(B, B);

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

		public Color4(float scalar)
		{
			R = scalar;
			G = scalar;
			B = scalar;
			A = scalar;
		}

		public Color3 RGB
		{
			get => new Color3(R, G, B);
			set { R = value.R; G = value.G; B = value.B; }
		}

		public Color3 GBA
		{
			get => new Color3(G, B, A);
			set { G = value.R; B = value.G; A = value.B; }
		}

		public Color2 RG
		{
			get => new Color2(R, G);
			set { R = value.R; G = value.G; }
		}

		public Color2 RB
		{
			get => new Color2(R, B);
			set { R = value.R; B = value.G; }
		}

		public Color2 BG
		{
			get => new Color2(B, G);
			set { B = value.R; G = value.G; }
		}

		public Color4 RRRR => new Color4(R, R, R, R);
		public Color4 GGGG => new Color4(G, G, G, G);
		public Color4 BBBB => new Color4(B, B, B, B);
		public Color4 AAAA => new Color4(A, A, A, A);

		public Color3 RRR => new Color3(R, R, R);
		public Color3 GGG => new Color3(G, G, G);
		public Color3 BBB => new Color3(B, B, B);
		public Color3 AAA => new Color3(A, A, A);

		public Color2 RR => new Color2(R, R);
		public Color2 GG => new Color2(G, G);
		public Color2 BB => new Color2(B, B);
		public Color2 AA => new Color2(A, A);

		public static implicit operator Vector4(Color4 color) => new Vector4(color.R, color.G, color.B, color.A);
	}

}
