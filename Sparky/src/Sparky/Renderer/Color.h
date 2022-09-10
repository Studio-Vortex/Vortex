#pragma once

#include "Sparky/Core/Math.h"

namespace Sparky {

	enum class Color
	{
		Red, Green, Blue, Yellow, White, Black,
		LightRed, LightGreen, LightBlue, LightYellow,
	};

	static Math::vec4 ColorToVec4(Color color)
	{
		switch (color)
		{
			case Sparky::Color::Red:          return Math::vec4(1.0f, 0.0f, 0.0f, 1.0f);
			case Sparky::Color::Green:        return Math::vec4(0.0f, 1.0f, 0.0f, 1.0f);
			case Sparky::Color::Blue:         return Math::vec4(0.0f, 0.0f, 1.0f, 1.0f);
			case Sparky::Color::Yellow:       return Math::vec4(1.0f, 1.0f, 0.0f, 1.0f);
			case Sparky::Color::White:        return Math::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			case Sparky::Color::Black:        return Math::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			case Sparky::Color::LightRed:     return Math::vec4(0.8f, 0.2f, 0.2f, 1.0f);
			case Sparky::Color::LightGreen:   return Math::vec4(0.2f, 0.8f, 0.2f, 1.0f);
			case Sparky::Color::LightBlue:    return Math::vec4(0.2f, 0.2f, 0.8f, 1.0f);
			case Sparky::Color::LightYellow:  return Math::vec4(0.8f, 0.8f, 0.2f, 1.0f);
		}

		return Math::vec4(1.0f);
	}

}