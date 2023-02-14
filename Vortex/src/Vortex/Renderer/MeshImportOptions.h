#pragma once

namespace Vortex {

	struct VORTEX_API MeshImportOptions
	{
		TransformComponent MeshTransformation;

		MeshImportOptions() = default;
		MeshImportOptions(const MeshImportOptions&) = default;

		inline bool operator==(const MeshImportOptions& other) const
		{
			return MeshTransformation.Translation == other.MeshTransformation.Translation &&
				MeshTransformation.GetRotationEuler() == other.MeshTransformation.GetRotationEuler() &&
				MeshTransformation.Scale == other.MeshTransformation.Scale;
		}
	};

}
