#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/System/IAssetSystem.h"
#include "Vortex/System/ISystem.h"

#include <unordered_map>

namespace Vortex {

	class VORTEX_API SystemManager
	{
	public:
		static void OnContextSceneCreated(Scene* context);
		static void OnContextSceneDestroyed(Scene* context);

		static void OnRuntimeStart(Scene* context);
		static void OnRuntimeScenePaused(Scene* context);
		static void OnRuntimeSceneResumed(Scene* context);
		static void OnRuntimeStop(Scene* context);

		static void OnGuiRender();

		/// Asset System Functions

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
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "UnRegisterAssetSystem only works with types derived from IAssetSystem!");

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
			{
				return nullptr;
			}

			return s_AssetSystems[assetType].As<TSystemType>();
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static bool ContainsAssetSystem()
		{
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "ContainsAssetSystem only works with types derived from IAssetSystem!");

			AssetType assetType = TSystemType::GetStaticType();
			return s_AssetSystems.contains(assetType);
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static bool IsAssetSystemEnabled()
		{
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "IsAssetSystemEnabled only works with types derived from IAssetSystem!");

			AssetType assetType = TSystemType::GetStaticType();
			VX_CORE_ASSERT(s_EnabledAssetSystems.contains(assetType), "AssetType not found!");

			return (bool)s_EnabledAssetSystems[assetType];
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static void SetAssetSystemEnabled(bool enabled)
		{
			static_assert(std::is_base_of<IAssetSystem, TSystemType>::value, "SetAssetSystemEnabled only works with types derived from IAssetSystem!");

			AssetType assetType = TSystemType::GetStaticType();

 			s_EnabledAssetSystems[assetType] = (uint8_t)enabled;
		}

		/// System Functions

		template <typename TSystemType>
		VX_FORCE_INLINE static SharedReference<ISystem> RegisterSystem()
		{
			static_assert(std::is_base_of<ISystem, TSystemType>::value, "RegisterSystem only works with types derived from ISystem!");

			SystemType systemType = TSystemType::GetStaticType();

			VX_CORE_ASSERT(!ContainsSystem<TSystemType>(), "System was already registered for SystemType!");
			SharedReference<ISystem> system = SharedReference<TSystemType>::Create();
			s_Systems[systemType] = system;
			SetSystemEnabled<TSystemType>(true);

			system->Init();

			return system;
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static void UnRegisterSystem()
		{
			static_assert(std::is_base_of<ISystem, TSystemType>::value, "UnRegisterSystem only works with types derived from ISystem!");

			SystemType systemType = TSystemType::GetStaticType();

			VX_CORE_ASSERT(ContainsSystem<TSystemType>(), "No System was found for SystemType!");
			SharedReference<ISystem> system = s_Systems[systemType];
			SetSystemEnabled<TSystemType>(false);

			system->Shutdown();

			s_Systems.erase(systemType);
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static SharedReference<ISystem> GetSystem()
		{
			static_assert(std::is_base_of<ISystem, TSystemType>::value, "GetSystem only works with types derived from ISystem!");

			SystemType systemType = TSystemType::GetStaticType();

			VX_CORE_ASSERT(ContainsSystem<TSystemType>(), "No System was found for SystemType!");
			if (!s_Systems.contains(systemType))
			{
				return nullptr;
			}

			return s_Systems[systemType].As<TSystemType>();
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static bool ContainsSystem()
		{
			static_assert(std::is_base_of<ISystem, TSystemType>::value, "ContainsSystem only works with types derived from ISystem!");

			SystemType systemType = TSystemType::GetStaticType();
			return s_Systems.contains(systemType);
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static bool IsSystemEnabled()
		{
			static_assert(std::is_base_of<ISystem, TSystemType>::value, "IsSystemEnabled only works with types derived from ISystem!");

			SystemType systemType = TSystemType::GetStaticType();
			VX_CORE_ASSERT(s_EnabledSystems.contains(systemType), "SystemType not found!");

			return (bool)s_EnabledSystems[systemType];
		}

		template <typename TSystemType>
		VX_FORCE_INLINE static bool SetSystemEnabled(bool enabled)
		{
			static_assert(std::is_base_of<ISystem, TSystemType>::value, "SetSystemEnabled only works with types derived from ISystem!");

			SystemType systemType = TSystemType::GetStaticType();

			s_EnabledSystems[systemType] = (uint8_t)enabled;
		}

	private:
		inline static std::unordered_map<AssetType, SharedReference<IAssetSystem>> s_AssetSystems;
		inline static std::unordered_map<AssetType, uint8_t> s_EnabledAssetSystems;

		inline static std::unordered_map<SystemType, SharedReference<ISystem>> s_Systems;
		inline static std::unordered_map<SystemType, uint8_t> s_EnabledSystems;

	private:
		friend class SystemManagerPanel;
	};

}
