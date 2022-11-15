#include "sppch.h"
#include "Project.h"

namespace Sparky {

	Project::Project(const ProjectProperties& props)
		: m_Properties(props) { }

	SharedRef<Project> Project::Create(const ProjectProperties& props)
	{
		return CreateShared<Project>(props);
	}

}
