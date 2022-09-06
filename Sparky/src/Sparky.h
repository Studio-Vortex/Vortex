#pragma once

#include "Sparky/Application.h"
#include "Sparky/Layer.h"
#include "Sparky/Log.h"

#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Events/KeyEvent.h"
#include "Sparky/Events/MouseEvent.h"

#include "Sparky/Input.h"
#include "Sparky/KeyCodes.h"
#include "Sparky/MouseButtonCodes.h"

#include "Sparky/Math.h"
using namespace Sparky::Math;

#include <imgui.h>
#include "Sparky/Gui/GuiLayer.h"

// Re Exporting ImGui as Gui in Client
namespace Gui = ImGui;

/// --- Entry Point --------------------------
#include "Sparky/EntryPoint.h"