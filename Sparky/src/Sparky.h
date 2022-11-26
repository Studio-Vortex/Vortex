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

/// Audio
#include "Sparky/Audio/AudioSource.h"
#include "Sparky/Audio/AudioListener.h"

/// Physics
#include "Sparky/Physics/Physics.h"
#include "Sparky/Physics/Physics2D.h"

/// Project
#include "Sparky/Project/Project.h"

/// Scene
#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scene/ScriptableEntity.h"
#include "Sparky/Scene/Components.h"
/// ---------------------------------------------------

/// Input & Events
#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Events/KeyEvent.h"
#include "Sparky/Events/MouseEvent.h"

#include "Sparky/Core/Input.h"
#include "Sparky/Core/KeyCodes.h"
#include "Sparky/Core/MouseCodes.h"
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
#include "Sparky/Renderer/ParticleEmitter.h"
#include "Sparky/Renderer/Model.h"
#include "Sparky/Renderer/Material.h"
#include "Sparky/Renderer/LightSource.h"
#include "Sparky/Renderer/Skybox.h"
#include "Sparky/Renderer/Font/Font.h"

#include "Sparky/Renderer/OrthographicCamera.h"
/// ---------------------------------------------------
