#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/System/IAssetSystem.h"

#include <unordered_map>

namespace Vortex {

	class SystemManager
	{
	public:
		static void OnContextSceneCreated(Scene* context);
		static void OnContextSceneDestroyed(Scene* context);

		static void OnRuntimeStart(Scene* context);
		static void OnRuntimeScenePaused(Scene* context);
		static void OnRuntimeSceneResumed(Scene* context);
		static void OnRuntimeStop(Scene* context);

		static void OnGuiRender();

		template <typename TSystemType>
		VX_FORCE_INLINE static SharedReference<IAssetSystem> RegisterAssetSystem()
		{
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "RegisterAssetSystem only works with types derived from IAssetSystem!");

			AssetType assetType = TSystemType::GetStaticType();

			VX_CORE_ASSERT(!ContainsAssetSystem<TSystemType>(), "AssetSystem was already registered for AssetType!");
			SharedReference<IAssetSystem> assetSystem = SharedReference<TSystemType>::Create();
			s_AssetSystems[assetType] = assetSystem;
			SetAssetSystemEnabled<TSystemType>(true);

			assetSystem->Init();

			return assetSystem;
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static void UnRegisterAssetSystem()
		{
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "UnregisterAssetSystem only works with types derived from IAssetSystem!");

			AssetType assetType = TSystemType::GetStaticType();

			VX_CORE_ASSERT(ContainsAssetSystem<TSystemType>(), "No AssetSystem was found for AssetType!");
			SharedReference<IAssetSystem> assetSystem = s_AssetSystems[assetType];
			SetAssetSystemEnabled<TSystemType>(false);

			assetSystem->Shutdown();

			s_AssetSystems.erase(assetType);
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static SharedReference<TSystemType> GetAssetSystem()
		{
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "GetAssetSystem only works with types derived from IAssetSystem!");

			AssetType assetType = TSystemType::GetStaticType();

			VX_CORE_ASSERT(ContainsAssetSystem<TSystemType>(), "No AssetSystem was found for AssetType!");
			if (!s_AssetSystems.contains(assetType))
				return nullptr;

			return s_AssetSystems[assetType].As<TSystemType>();
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static bool ContainsAssetSystem()
		{
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "ContainsSystem only works with types derived from IAssetSystem!");

			AssetType assetType = TSystemType::GetStaticType();
			return s_AssetSystems.contains(assetType);
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static bool IsAssetSystemEnabled()
		{
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "IsSystemEnabled only works with types derived from IAssetSystem!");

			AssetType assetType = TSystemType::GetStaticType();
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "AssetType not found!");

			return (bool)s_EnabledSystems[assetType];
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static void SetAssetSystemEnabled(bool enabled)
		{
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "SetSystemEnabled only works with types derived from IAssetSystem!");

			AssetType assetType = TSystemType::GetStaticType();

 			s_EnabledSystems[assetType] = (uint8_t)enabled;
		}

	private:
		inline static std::unordered_map<AssetType, SharedReference<IAssetSystem>> s_AssetSystems;
		inline static std::unordered_map<AssetType, uint8_t> s_EnabledSystems;

	private:
		friend class SystemManagerPanel;
	};

}
