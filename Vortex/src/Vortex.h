#pragma once

/// Core
#include "Vortex/Core/Application.h"
#include "Vortex/Core/Platform.h"
#include "Vortex/Core/Layer.h"
#include "Vortex/Core/Log.h"
#include "Vortex/Core/Buffer.h"
#include "Vortex/Core/Window.h"
#include "Vortex/Core/Platform.h"
#include "Vortex/Core/TimeStep.h"
#include "Vortex/System/SystemManager.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"
/// ---------------------------------------------------

/// Events
#include "Vortex/Events/WindowEvent.h"
#include "Vortex/Events/KeyEvent.h"
#include "Vortex/Events/MouseEvent.h"
/// ---------------------------------------------------

/// Input
#include "Vortex/Core/Input/Input.h"
#include "Vortex/Core/Input/Gamepad.h"
#include "Vortex/Core/Input/KeyCodes.h"
#include "Vortex/Core/Input/MouseCodes.h"
/// ---------------------------------------------------

/// Math
#include "Vortex/Core/Math/Math.h"
#include "Vortex/Core/Math/Ray.h"
#include "Vortex/Core/Math/AABB.h"
#include "Vortex/Core/Math/Noise.h"
/// ---------------------------------------------------

/// Networking
#include "Vortex/Network/Networking.h"
/// ---------------------------------------------------

/// Gui
#include "Vortex/Gui/GuiLayer.h"
#include "Vortex/UI/UI.h"
/// ---------------------------------------------------

/// Project
#include "Vortex/Project/Project.h"
/// ---------------------------------------------------

/// Asset Manager
#include "Vortex/Asset/AssetManager.h"
#include "Vortex/Asset/Asset.h"
/// ---------------------------------------------------

/// AI
#include "Vortex/AI/AStarPathFinder.h"
/// ---------------------------------------------------

/// Audio
#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Audio/PlaybackDevice.h"
#include "Vortex/Audio/AudioListener.h"
#include "Vortex/Audio/ListenerDevice.h"
/// ---------------------------------------------------

/// Animation
#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"
/// ---------------------------------------------------

/// Physics
#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/3D/PhysXTypes.h"
#include "Vortex/Physics/3D/PhysicsMaterial.h"
#include "Vortex/Physics/3D/PhysicsShapes.h"
#include "Vortex/Physics/2D/Physics2D.h"
/// ---------------------------------------------------

/// Scene
#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Prefab.h"
#include "Vortex/Scene/ScriptableEntity.h"
#include "Vortex/Scene/Components.h"
/// ---------------------------------------------------

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
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"
/// ---------------------------------------------------

/// Utilities
#include "Vortex/Utils/FileDialogue.h"
#include "Vortex/Utils/FileSystem.h"
#include "Vortex/Utils/StringUtils.h"
#include "Vortex/Utils/Random.h"
#include "Vortex/Utils/Time.h"
/// ---------------------------------------------------
