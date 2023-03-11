#pragma once

class FastNoiseLite;

namespace Vortex {

	enum class NoiseType
	{
		Cellular,
		OpenSimplex2,
		OpenSimplex2S,
		Perlin,
		Value,
		ValueCubic,
	};

	class Noise
	{
	public:
		Noise(int seed = 8, NoiseType type = NoiseType::Perlin);
		~Noise();

		float GetFrequency() const;
		void SetFrequency(float frequency);

		int GetFractalOctaves() const;
		void SetFractalOctaves(int octaves);

		float GetFractalLacunarity() const;
		void SetFractalLacunarity(float lacunarity);

		float GetFractalGain() const;
		void SetFractalGain(float gain);

		float Get(float x, float y);

		static void SetSeed(int seed);
		static float PerlinNoise(float x, float y);

	private:
		FastNoiseLite* m_FastNoise = nullptr;
		float m_Frequency = 0.0f;
		int m_FractalOctaves = 0;
		float m_FractalLacunarity = 0.0f;
		float m_FractalGain = 0.0f;
	};

}
