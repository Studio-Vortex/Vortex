#pragma once

#include "Core.h"

namespace Sparky {
	class SPARKY_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	private:
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}