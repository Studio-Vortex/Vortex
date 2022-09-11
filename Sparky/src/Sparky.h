#pragma once

/// Core
#include "Sparky/Core/Application.h"
#include "Sparky/Core/Layer.h"
#include "Sparky/Core/Log.h"
#include "Sparky/Core/TimeStep.h"
/// ---------------------------------------------------

/// Math
#include "Sparky/Core/Math.h"
namespace Math = Sparky::Math;
/// ---------------------------------------------------

/// Gui
#include "Sparky/Gui/GuiLayer.h"
#include <imgui.h>
// Re Exporting ImGui as Gui in Client
namespace Gui = ImGui;
/// ---------------------------------------------------

/// Input & Events
#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Events/KeyEvent.h"
#include "Sparky/Events/MouseEvent.h"

#include "Sparky/Core/Input.h"
#include "Sparky/Core/KeyCodes.h"
#include "Sparky/Core/MouseButtonCodes.h"
#include "Sparky/Renderer/OrthographicCameraController.h"
/// ---------------------------------------------------

/// Graphics
#include "Sparky/Renderer/Renderer.h"
#include "Sparky/Renderer/Renderer2D.h"
#include "Sparky/Renderer/RenderCommand.h"

#include "Sparky/Renderer/Color.h"

#include "Sparky/Renderer/Buffer.h"
#include "Sparky/Renderer/Shader.h"
#include "Sparky/Renderer/Texture.h"
#include "Sparky/Renderer/SubTexture2D.h"
#include "Sparky/Renderer/Framebuffer.h"
#include "Sparky/Renderer/VertexArray.h"

#include "Sparky/Renderer/OrthographicCamera.h"
/// ---------------------------------------------------
