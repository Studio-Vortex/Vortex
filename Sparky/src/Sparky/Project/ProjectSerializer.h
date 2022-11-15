#include "Sparky/Core/Base.h"
#include "Sparky/Project/Project.h"

#include <string>

namespace Sparky {

	class ProjectSerializer
	{
	public:
		ProjectSerializer(const SharedRef<Project>& project);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);

	private:
		SharedRef<Project> m_Project = nullptr;
	};

}
