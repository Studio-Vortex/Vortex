using System;

namespace Vortex {

	public enum NoiseType
	{
		Cellular,
		OpenSimplex2,
		OpenSimplex2S,
		Perlin,
		Value,
		ValueCubic,
	}

	public class Noise
	{
		private IntPtr m_UnmanagedInstance;

		public Noise(int seed = 8, NoiseType type = NoiseType.Perlin)
		{
			m_UnmanagedInstance = InternalCalls.Noise_Constructor(seed, type);
		}

		~Noise()
		{
			InternalCalls.Noise_Destructor(m_UnmanagedInstance);
		}

		public float Frequency
		{
			get => InternalCalls.Noise_GetFrequency(m_UnmanagedInstance);
			set => InternalCalls.Noise_SetFrequency(m_UnmanagedInstance, value);
		}

		public int Octaves
		{
			get => InternalCalls.Noise_GetFractalOctaves(m_UnmanagedInstance);
			set => InternalCalls.Noise_SetFractalOctaves(m_UnmanagedInstance, value);
		}

		public float Lacunarity
		{
			get => InternalCalls.Noise_GetFractalLacunarity(m_UnmanagedInstance);
			set => InternalCalls.Noise_SetFractalLacunarity(m_UnmanagedInstance, value);
		}

		public float Gain
		{
			get => InternalCalls.Noise_GetFractalGain(m_UnmanagedInstance);
			set => InternalCalls.Noise_SetFractalGain(m_UnmanagedInstance, value);
		}

		public float Get(float x, float y)
		{
			return InternalCalls.Noise_GetVec2(m_UnmanagedInstance, x, y);
		}

		public float Get(float x, float y, float z)
		{
			return InternalCalls.Noise_GetVec3(m_UnmanagedInstance, x, y, z);
		}

		public float Get(Vector2 position)
		{
			return Get(position.X, position.Y);
		}

		public float Get(Vector3 position)
		{
			return Get(position.X, position.Y, position.Z);
		}

		public static void SetSeed(int seed)
		{
			InternalCalls.Noise_SetSeed(seed);
		}

		public static float PerlinNoise(float x, float y)
		{
			return InternalCalls.Noise_PerlinNoiseVec2(x, y);
		}

		public static float PerlinNoise(float x, float y, float z)
		{
			return InternalCalls.Noise_PerlinNoiseVec3(x, y, z);
		}

		public static float PerlinNoise(Vector2 position)
		{
			return PerlinNoise(position.X, position.Y);
		}

		public static float PerlinNoise(Vector3 position)
		{
			return PerlinNoise(position.X, position.Y, position.Z);
		}
	}

}
