#pragma once

/// Core
#include "Vortex/Core/Application.h"
#include "Vortex/Core/Platform.h"
#include "Vortex/Core/Layer.h"
#include "Vortex/Core/Log.h"
#include "Vortex/Core/String.h"
#include "Vortex/Core/Buffer.h"
#include "Vortex/Core/Window.h"
#include "Vortex/Core/Platform.h"
#include "Vortex/Core/Timer.h"
#include "Vortex/Core/TimeStep.h"
#include "Vortex/System/SystemManager.h"
#include "Vortex/ReferenceCounting/SharedRef.h"
#include "Vortex/ReferenceCounting/RefCounted.h"
/// ---------------------------------------------------

/// Vortex STL
#include "Vortex/stl/option.h"
#include "Vortex/stl/result.h"
/// ---------------------------------------------------

/// Events
#include "Vortex/Events/WindowEvent.h"
#include "Vortex/Events/KeyEvent.h"
#include "Vortex/Events/MouseEvent.h"
/// ---------------------------------------------------

/// Input
#include "Vortex/Input/Input.h"
#include "Vortex/Input/Gamepad.h"
#include "Vortex/Input/KeyCodes.h"
#include "Vortex/Input/MouseCodes.h"
/// ---------------------------------------------------

/// Math
#include "Vortex/Math/Math.h"
#include "Vortex/Math/Ray.h"
#include "Vortex/Math/AABB.h"
#include "Vortex/Math/Noise.h"
/// ---------------------------------------------------

/// Module
#include "Vortex/Module/Module.h"
/// ---------------------------------------------------

/// Networking
#include "Vortex/Network/Networking.h"
#include "Vortex/Network/Server.h"
#include "Vortex/Network/Socket.h"
/// ---------------------------------------------------

/// UI
#include "Vortex/Gui/GuiLayer.h"
#include "Vortex/Editor/UI/UI.h"
#include "Vortex/Editor/UI/UI_Widgets.h"
/// ---------------------------------------------------

/// Project
#include "Vortex/Project/Project.h"
#include "Vortex/Project/ProjectType.h"
/// ---------------------------------------------------

/// Asset Manager
#include "Vortex/Asset/Asset.h"
#include "Vortex/Asset/AssetTypes.h"
#include "Vortex/Asset/AssetManager.h"
/// ---------------------------------------------------

/// AI
#include "Vortex/AI/AStarPathFinder.h"
/// ---------------------------------------------------

/// Audio
#include "Vortex/Audio/Audio.h"
#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Audio/AudioTypes.h"
#include "Vortex/Audio/AudioSystem.h"
/// ---------------------------------------------------

/// Animation
#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"
/// ---------------------------------------------------

/// Physics
#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/3D/PhysicsScene.h"
#include "Vortex/Physics/3D/PhysicsMaterial.h"
#include "Vortex/Physics/3D/PhysicsShapes.h"
#include "Vortex/Physics/3D/PhysicsTypes.h"

#include "Vortex/Physics/2D/Physics2D.h"
/// ---------------------------------------------------

/// Scene
#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"
#include "Vortex/Scene/Prefab.h"
#include "Vortex/Scene/SceneCamera.h"
#include "Vortex/Scene/ScriptableActor.h"
#include "Vortex/Scene/Components.h"
/// ---------------------------------------------------

/// Scripting
#include "Vortex/Scripting/ScriptClass.h"
#include "Vortex/Scripting/ScriptInstance.h"
#include "Vortex/Scripting/ScriptField.h"
/// ---------------------------------------------------

/// Rendering
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/RenderCommand.h"

#include "Vortex/Renderer/Camera.h"
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

#include "Vortex/Renderer/ParticleSystem/ParticleSystem.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"
#include "Vortex/Renderer/ParticleSystem/Particle.h"
/// ---------------------------------------------------

/// Utilities
#include "Vortex/Utils/FileDialogue.h"
#include "Vortex/Utils/FileSystem.h"
#include "Vortex/Utils/Random.h"
#include "Vortex/Utils/Time.h"
/// ---------------------------------------------------
