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
#include <cmath>
#include <any>

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

#include "Sparky/Core/Log.h"
#include "Sparky/Debug/Instrumentor.h"

#ifdef SP_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // SP_PLATFORM_WINDOWS