#pragma once

#include "Sparky/Scene/Scene.h"
#include "Sparky/Core/Base.h"

namespace Sparky {

	class SceneSerializer
	{
	public:
		SceneSerializer(const SharedRef<Scene>& scene);
		
		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);

	private:
		SharedRef<Scene> m_Scene;
	};

}
