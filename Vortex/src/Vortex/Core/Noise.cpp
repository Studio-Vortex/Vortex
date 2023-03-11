#include "vxpch.h"
#include "Noise.h"

#include "Cpp/FastNoiseLite.h"

namespace Vortex {

	namespace Utils {

		FastNoiseLite::NoiseType VortexNoiseTypeToFastNoiseType(NoiseType type)
		{
			switch (type)
			{
				case Vortex::NoiseType::Cellular:      return FastNoiseLite::NoiseType_Cellular;
				case Vortex::NoiseType::OpenSimplex2:  return FastNoiseLite::NoiseType_OpenSimplex2;
				case Vortex::NoiseType::OpenSimplex2S: return FastNoiseLite::NoiseType_OpenSimplex2S;
				case Vortex::NoiseType::Perlin:        return FastNoiseLite::NoiseType_Perlin;
				case Vortex::NoiseType::Value:         return FastNoiseLite::NoiseType_Value;
				case Vortex::NoiseType::ValueCubic:    return FastNoiseLite::NoiseType_ValueCubic;
			}

			VX_CORE_ASSERT(false, "Unknown Noise Type!");
			return FastNoiseLite::NoiseType_Perlin;
		}

	}

	static FastNoiseLite s_FastNoise;

	Noise::Noise(int seed, NoiseType type)
	{
		m_FastNoise = new FastNoiseLite(seed);

		FastNoiseLite::NoiseType noiseType = Utils::VortexNoiseTypeToFastNoiseType(type);
		m_FastNoise->SetNoiseType(noiseType);
	}

	Noise::~Noise()
	{
		delete m_FastNoise;
	}

	float Noise::GetFrequency() const
	{
		return m_Frequency;
	}

	void Noise::SetFrequency(float frequency)
	{
		m_Frequency = frequency;
		m_FastNoise->SetFrequency(m_Frequency);
	}

	int Noise::GetFractalOctaves() const
	{
		return m_FractalOctaves;
	}

	void Noise::SetFractalOctaves(int octaves)
	{
		m_FractalOctaves = octaves;
		m_FastNoise->SetFractalOctaves(m_FractalOctaves);
	}

	float Noise::GetFractalLacunarity() const
	{
		return m_FractalLacunarity;
	}

	void Noise::SetFractalLacunarity(float lacunarity)
	{
		m_FractalLacunarity = lacunarity;
		m_FastNoise->SetFractalLacunarity(m_FractalLacunarity);
	}

	float Noise::GetFractalGain() const
	{
		return m_FractalGain;
	}

	void Noise::SetFractalGain(float gain)
	{
		m_FractalGain = gain;
		m_FastNoise->SetFractalGain(m_FractalGain);
	}

	float Noise::Get(float x, float y)
	{
		return Get({ x, y });
	}

	float Noise::Get(const Math::vec2& position)
	{
		return m_FastNoise->GetNoise(position.x, position.y);
	}

	float Noise::Get(float x, float y, float z)
	{
		return Get({ x, y, z });
	}

	float Noise::Get(const Math::vec3& position)
	{
		return m_FastNoise->GetNoise(position.x, position.y, position.z);
	}

	void Noise::SetSeed(int seed)
	{
		s_FastNoise.SetSeed(seed);
	}

	float Noise::PerlinNoise(float x, float y)
	{
		return PerlinNoise({ x, y });
	}

	float Noise::PerlinNoise(const Math::vec2& position)
	{
		s_FastNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

		// Between -1 and 1
		const float noise = s_FastNoise.GetNoise(position.x, position.y);
		return noise;
	}

	float Noise::PerlinNoise(float x, float y, float z)
	{
		return PerlinNoise({ x, y, z });
	}

	float Noise::PerlinNoise(const Math::vec3& position)
	{
		s_FastNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

		// Between -1 and 1
		const float noise = s_FastNoise.GetNoise(position.x, position.y, position.z);
		return noise;
	}

}
