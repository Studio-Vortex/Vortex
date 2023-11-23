#include "vxpch.h"
#include "ParticleSystem.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"

#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

namespace Vortex {

	struct ParticleSystemInternalData
	{
		using ParticleEmitterData = std::unordered_map<UUID, AssetHandle>;
		Scene* ActiveScene = nullptr;
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
		
		s_Data.ActiveScene = context;
		s_Data.ParticleData.clear();
	}

	void ParticleSystem::OnContextSceneDestroyed()
	{
		s_Data.ActiveScene = nullptr;
		s_Data.ParticleData.clear();
	}

	void ParticleSystem::CreateAsset(Entity& entity)
	{
		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid scene!");
		VX_CORE_ASSERT(entity.HasComponent<ParticleEmitterComponent>(), "Entity doesn't have particle emitter component!");

		std::string particleSystemDir = "Cache/ParticleSystem";
		if (!FileSystem::Exists(particleSystemDir))
			FileSystem::CreateDirectoriesV(Project::GetAssetDirectory() / particleSystemDir);

		std::string filename = "ParticleSystem";
		std::string filepath = filename + ".vparticle";

		ParticleEmitterComponent& pmc = entity.GetComponent<ParticleEmitterComponent>();

		SharedReference<ParticleEmitter> particleEmitter = Project::GetEditorAssetManager()->CreateNewAsset<ParticleEmitter>(particleSystemDir, filepath, ParticleEmitterProperties{});
		pmc.EmitterHandle = particleEmitter->Handle;
		particleEmitter->SetName(filename);
	}

	void ParticleSystem::DestroyAsset(Entity& entity)
	{
		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid scene!");
		VX_CORE_ASSERT(entity.HasComponent<ParticleEmitterComponent>(), "Entity doesn't have particle emitter component!");

		VX_CORE_ASSERT(s_Data.ParticleData.contains(entity.GetUUID()), "Entity was not found in scene particle emitter map!");

		s_Data.ParticleData.erase(entity.GetUUID());
	}

	void ParticleSystem::OnRuntimeStart()
	{
	}

	void ParticleSystem::OnUpdateRuntime(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		for (auto& [entityUUID, assetHandle] : s_Data.ParticleData)
		{
			Entity entity = s_Data.ActiveScene->TryGetEntityWithUUID(entityUUID);
			if (!entity.IsActive())
				continue;

			auto& pmc = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
				continue;

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				continue;

			// Set the particle position to the entity's translation
			Math::vec3 entityTranslation = s_Data.ActiveScene->GetWorldSpaceTransform(entity).Translation;
			particleEmitter->GetProperties().Position = entityTranslation;
			particleEmitter->OnUpdate(delta);

			if (!particleEmitter->IsActive())
				continue;

			particleEmitter->EmitParticle();
		}
	}

	void ParticleSystem::OnRuntimeScenePaused()
	{
	}

	void ParticleSystem::OnRuntimeSceneResumed()
	{
	}

	void ParticleSystem::OnRuntimeStop()
	{
	}

	void ParticleSystem::OnGuiRender()
	{
	}

}
