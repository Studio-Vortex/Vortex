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
		std::unordered_map<Scene*, ParticleEmitterData> ActiveScenes;

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
		moduleProps.ModuleName = "Particle System";
		moduleProps.APIVersion = Version(1, 1, 0);
		moduleProps.RequiredModules = {};
		s_Data.Module.Init(moduleProps);

		Application::Get().AddModule(s_Data.Module);
	}

	void ParticleSystem::Shutdown()
	{
		s_Data.ActiveScenes.clear();

		Application::Get().RemoveModule(s_Data.Module);
		s_Data.Module.Shutdown();
	}

	void ParticleSystem::SubmitContextScene(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(!s_Data.ActiveScenes.contains(context), "Scene was already added to particle system!");

		s_Data.ActiveScenes[context] = ParticleSystemInternalData::ParticleEmitterData{};
	}

	void ParticleSystem::RemoveContextScene(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data.ActiveScenes.contains(context), "Invalid scene!");

		auto& particleData = s_Data.ActiveScenes[context];
		
		particleData.clear();

		s_Data.ActiveScenes.erase(context);
	}

	void ParticleSystem::CreateAsset(Entity& entity, Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data.ActiveScenes.contains(context), "Invalid Scene!");
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

	void ParticleSystem::DestroyAsset(Entity& entity, Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data.ActiveScenes.contains(context), "Invalid scene!");
		VX_CORE_ASSERT(entity.HasComponent<ParticleEmitterComponent>(), "Entity doesn't have particle emitter component!");

		auto& particleData = s_Data.ActiveScenes[context];
		VX_CORE_ASSERT(particleData.contains(entity.GetUUID()), "Entity was not found in scene particle emitter map!");

		particleData.erase(entity.GetUUID());
	}

	void ParticleSystem::OnRuntimeStart(Scene* context)
	{
	}

	void ParticleSystem::OnUpdateRuntime(Scene* context, TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data.ActiveScenes.contains(context), "Invalid scene!");

		auto& particleData = s_Data.ActiveScenes[context];

		for (auto& [entityUUID, assetHandle] : particleData)
		{
			Entity entity = context->TryGetEntityWithUUID(entityUUID);
			if (!entity.IsActive())
				continue;

			auto& pmc = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
				continue;

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				continue;

			// Set the particle position to the entity's translation
			Math::vec3 entityTranslation = context->GetWorldSpaceTransform(entity).Translation;
			particleEmitter->GetProperties().Position = entityTranslation;
			particleEmitter->OnUpdate(delta);

			if (!particleEmitter->IsActive())
				continue;

			particleEmitter->EmitParticle();
		}
	}

	void ParticleSystem::OnRuntimeScenePaused(Scene* context)
	{
	}

	void ParticleSystem::OnRuntimeSceneResumed(Scene* context)
	{
	}

	void ParticleSystem::OnRuntimeStop(Scene* context)
	{
	}

	void ParticleSystem::OnGuiRender()
	{
	}

}
