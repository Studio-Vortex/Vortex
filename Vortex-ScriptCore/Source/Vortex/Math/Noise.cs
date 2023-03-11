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
			return InternalCalls.Noise_Get(m_UnmanagedInstance, x, y);
		}

		public static void SetSeed(int seed)
		{
			InternalCalls.Noise_SetSeed(seed);
		}

		public static float PerlinNoise(float x, float y)
		{
			return InternalCalls.Noise_PerlinNoise(x, y);
		}
	}

}
