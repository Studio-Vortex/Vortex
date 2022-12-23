#pragma once

/// Core
#include "Vortex/Core/Application.h"
#include "Vortex/Core/Layer.h"
#include "Vortex/Core/Log.h"
#include "Vortex/Core/TimeStep.h"
/// ---------------------------------------------------

/// Math
#include "Vortex/Core/Math.h"
namespace Math = Vortex::Math;
/// ---------------------------------------------------

/// Gui
#include "Vortex/Gui/GuiLayer.h"
#include "Vortex/UI/UI.h"
#include <imgui.h>
// Re Exporting ImGui as Gui in Client
namespace Gui = ImGui;
/// ---------------------------------------------------

/// AI
#include "Vortex/AI/AStarPathFinder.h"

/// Audio
#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Audio/AudioListener.h"

/// Physics
#include "Vortex/Physics/Physics.h"
#include "Vortex/Physics/Physics2D.h"

/// Project
#include "Vortex/Project/Project.h"

/// Scene
#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Prefab.h"
#include "Vortex/Scene/ScriptableEntity.h"
#include "Vortex/Scene/Components.h"
/// ---------------------------------------------------

/// Events + Input
#include "Vortex/Events/ApplicationEvent.h"
#include "Vortex/Events/KeyEvent.h"
#include "Vortex/Events/MouseEvent.h"

#include "Vortex/Core/Input.h"
#include "Vortex/Core/KeyCodes.h"
#include "Vortex/Core/MouseCodes.h"
/// ---------------------------------------------------

/// Animation
#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"

/// Rendering
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/RenderCommand.h"

#include "Vortex/Renderer/Color.h"

#include "Vortex/Renderer/Buffer.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/SubTexture2D.h"
#include "Vortex/Renderer/Framebuffer.h"
#include "Vortex/Renderer/VertexArray.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Renderer/Model.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Font/Font.h"
/// ---------------------------------------------------
