#pragma once

#include "Sparky/Core.h"

#include <string>

namespace Sparky {

	class SPARKY_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Enable() const = 0;
		virtual void Disable() const = 0;

		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);
	};

}