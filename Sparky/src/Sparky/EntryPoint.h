#pragma once

#include "Application.h"

#ifdef SP_PLATFORM_WINDOWS

	extern std::unique_ptr<Sparky::Application> Sparky::CreateApplication();

	int main(int argc, char* argv[])
	{
		Sparky::Log::Init();

		auto app = Sparky::CreateApplication();
		app->Run();
	}

#endif // SP_PLATFORM_WINDOWS
