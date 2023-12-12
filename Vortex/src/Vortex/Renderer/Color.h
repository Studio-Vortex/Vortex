#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Math/Math.h"

namespace Vortex {

	enum class VORTEX_API Color
	{
		Red,
		Green,
		Blue,
		Purple,
		Yellow,
		Orange,
		White,
		Black,
		LightRed,
		LightGreen,
		LightBlue,
		LightYellow,
	};

	VORTEX_API static Math::vec4 ColorToVec4(Color color)
	{
		switch (color)
		{
			case Color::Red:         return Math::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			case Color::Green:       return Math::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			case Color::Blue:        return Math::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			case Color::Purple:      return Math::vec4(0.6f, 0.1f, 0.9f, 1.0f);
			case Color::Yellow:      return Math::vec4(1.0f, 1.0f, 0.0f, 1.0f);
			case Color::Orange:      return Math::vec4(1.0f, 0.4f, 0.0f, 1.0f);
			case Color::White:       return Math::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			case Color::Black:       return Math::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			case Color::LightRed:    return Math::vec4(0.8f, 0.2f, 0.2f, 1.0f);
			case Color::LightGreen:  return Math::vec4(0.2f, 0.8f, 0.2f, 1.0f);
			case Color::LightBlue:   return Math::vec4(0.2f, 0.2f, 0.8f, 1.0f);
			case Color::LightYellow: return Math::vec4(0.8f, 0.8f, 0.2f, 1.0f);
		}

		return Math::vec4(1.0f);
	}

}
