#pragma once

/// Core
#include "Sparky/Application.h"
#include "Sparky/Layer.h"
#include "Sparky/Log.h"
#include "Sparky/Core/TimeStep.h"

#include "Sparky/Gui/GuiLayer.h"

#include <imgui.h>
// Re Exporting ImGui as Gui in Client
namespace Gui = ImGui;
/// ---------------------------------------------------

/// Input & Events
#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Events/KeyEvent.h"
#include "Sparky/Events/MouseEvent.h"

#include "Sparky/Input.h"
#include "Sparky/KeyCodes.h"
#include "Sparky/MouseButtonCodes.h"
/// ---------------------------------------------------

/// Math
#include "Sparky/Math.h"
/// ---------------------------------------------------

/// Graphics
#include "Sparky/Renderer/Renderer.h"
#include "Sparky/Renderer/RenderCommand.h"

#include "Sparky/Renderer/Buffer.h"
#include "Sparky/Renderer/Shader.h"
#include "Sparky/Renderer/VertexArray.h"

#include "Sparky/Renderer/OrthographicCamera.h"
/// ---------------------------------------------------

/// Entry Point
#include "Sparky/EntryPoint.h"
/// ---------------------------------------------------