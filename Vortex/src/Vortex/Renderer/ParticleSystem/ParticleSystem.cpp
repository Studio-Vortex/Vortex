#include "vxpch.h"
#include "ParticleSystem.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"

#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

namespace Vortex {

	struct ParticleSystemInternalData
	{
		using ParticleEmitterData = std::unordered_map<UUID, AssetHandle>;
		ParticleEmitterData ParticleData;

		SubModule Module;
	};

	static ParticleSystemInternalData s_Data;

    ParticleSystem::ParticleSystem()
		: IAssetSystem("Particle System")
    {
    }

    void ParticleSystem::Init()
	{
		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Particle-System";
		moduleProps.APIVersion = Version(1, 1, 0);
		moduleProps.RequiredModules = {};
		s_Data.Module.Init(moduleProps);

		Application::Get().AddModule(s_Data.Module);
	}

	void ParticleSystem::Shutdown()
	{
		s_Data.ParticleData.clear();

		Application::Get().RemoveModule(s_Data.Module);
		s_Data.Module.Shutdown();
	}

	void ParticleSystem::OnContextSceneCreated(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
		
		s_Data.ParticleData.clear();
	}

	void ParticleSystem::OnContextSceneDestroyed(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		s_Data.ParticleData.clear();
	}

	void ParticleSystem::CreateAsset(Actor& actor)
	{
		VX_CORE_ASSERT(actor.HasComponent<ParticleEmitterComponent>(), "Actor doesn't have particle emitter component!");

		std::string particleSystemDir = "Cache/ParticleSystem";
		if (!FileSystem::Exists(particleSystemDir))
			FileSystem::CreateDirectoriesV(Project::GetAssetDirectory() / particleSystemDir);

		std::string filename = "ParticleSystem";
		std::string filepath = filename + ".vparticle";

		ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();

		SharedReference<ParticleEmitter> particleEmitter = Project::GetEditorAssetManager()->CreateNewAsset<ParticleEmitter>(particleSystemDir, filepath, ParticleEmitterProperties{});
		pmc.EmitterHandle = particleEmitter->Handle;
		particleEmitter->SetName(filename);
	}

	void ParticleSystem::DestroyAsset(Actor& actor)
	{
		VX_CORE_ASSERT(actor.HasComponent<ParticleEmitterComponent>(), "Actor doesn't have particle emitter component!");

		VX_CORE_ASSERT(s_Data.ParticleData.contains(actor.GetUUID()), "Actor was not found in scene particle emitter map!");

		s_Data.ParticleData.erase(actor.GetUUID());
	}

	void ParticleSystem::OnRuntimeStart(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
	}

	void ParticleSystem::OnUpdateRuntime(Scene* context, TimeStep delta)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [actorUUID, assetHandle] : s_Data.ParticleData)
		{
			Actor actor = context->TryGetActorWithUUID(actorUUID);
			if (!actor.IsActive())
				continue;

			auto& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
				continue;

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				continue;

			// Set the particle position to the actor's translation
			const Math::vec3 actorTranslation = context->GetWorldSpaceTransform(actor).Translation;
			particleEmitter->GetProperties().Position = actorTranslation;
			particleEmitter->OnUpdate(delta);

			if (!particleEmitter->IsActive())
				continue;

			particleEmitter->EmitParticle();
		}
	}

	void ParticleSystem::OnRuntimeScenePaused(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
	}

	void ParticleSystem::OnRuntimeSceneResumed(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
	}

	void ParticleSystem::OnRuntimeStop(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
	}

	void ParticleSystem::OnGuiRender()
	{
	}

}
