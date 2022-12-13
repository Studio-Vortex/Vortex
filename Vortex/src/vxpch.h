#pragma once

#include <functional>
#include <filesystem>
#include <algorithm>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <utility>
#include <cstring>
#include <memory>
#include <random>
#include <cmath>
#include <any>

#include <thread>
#include <mutex>
#include <atomic>
#include <future>

#include <string>
#include <sstream>
#include <bitset>
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "Vortex/Core/Log.h"
#include "Vortex/Debug/Instrumentor.h"

#ifdef VX_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // VX_PLATFORM_WINDOWS
