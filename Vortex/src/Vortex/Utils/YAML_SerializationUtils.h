#pragma once

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/Math/Math.h"

#include "Vortex/Scene/Components.h"

#include "Vortex/Audio/AttenuationModel.h"

#include "Vortex/Physics/3D/PhysXTypes.h"

#include <yaml-cpp/yaml.h>

#include <string>

namespace Vortex {

	#define VX_SERIALIZE_PROPERTY(name, value, outputNode) outputNode << YAML::Key << #name << YAML::Value << value
	#define VX_DESERIALIZE_PROPERTY(name, type, value, inputNode) value = inputNode[#name].as<type>()

	namespace Utils {

		static std::string LightTypeToString(LightType lightType)
		{
			switch (lightType)
			{
				case LightType::Directional:  return "Directional";
				case LightType::Point:        return "Point";
				case LightType::Spot:         return "Spot";
			}

			VX_CORE_ASSERT(false, "Unknown Light Type!");
			return {};
		}

		static LightType LightTypeFromString(const std::string& lightTypeString)
		{
			if (lightTypeString == "Directional")  return LightType::Directional;
			if (lightTypeString == "Point")        return LightType::Point;
			if (lightTypeString == "Spot")         return LightType::Spot;

			VX_CORE_ASSERT(false, "Unknown Light Type!");
			return LightType::Directional;
		}

		static std::string MeshTypeToString(MeshType meshType)
		{
			switch (meshType)
			{
				case MeshType::Cube:     return "Cube";
				case MeshType::Sphere:   return "Sphere";
				case MeshType::Capsule:  return "Capsule";
				case MeshType::Cone:     return "Cone";
				case MeshType::Cylinder: return "Cylinder";
				case MeshType::Plane:    return "Plane";
				case MeshType::Torus:    return "Torus";
				case MeshType::Custom:   return "Custom";
			}

			VX_CORE_ASSERT(false, "Unknown Mesh Type!");
			return {};
		}

		static MeshType MeshTypeFromString(const std::string& meshTypeString)
		{
			if (meshTypeString == "Cube")     return MeshType::Cube;
			if (meshTypeString == "Sphere")   return MeshType::Sphere;
			if (meshTypeString == "Capsule")  return MeshType::Capsule;
			if (meshTypeString == "Cone")     return MeshType::Cone;
			if (meshTypeString == "Cylinder") return MeshType::Cylinder;
			if (meshTypeString == "Plane")    return MeshType::Plane;
			if (meshTypeString == "Torus")    return MeshType::Torus;
			if (meshTypeString == "Custom")   return MeshType::Custom;

			VX_CORE_ASSERT(false, "Unknown Mesh Type!");
			return MeshType::Cube;
		}

		static std::string AttenuationModelTypeToString(AttenuationModel attenuationModel)
		{
			switch (attenuationModel)
			{
				case AttenuationModel::None:        return "None";
				case AttenuationModel::Inverse:     return "Inverse";
				case AttenuationModel::Linear:      return "Linear";
				case AttenuationModel::Exponential: return "Exponential";
			}

			VX_CORE_ASSERT(false, "Unknown Attenutation Model!");
			return "None";
		}

		static AttenuationModel AttenuationModelTypeFromString(const std::string& attenuationModel)
		{
			if (attenuationModel == "None")        return AttenuationModel::None;
			if (attenuationModel == "Inverse")     return AttenuationModel::Inverse;
			if (attenuationModel == "Linear")      return AttenuationModel::Linear;
			if (attenuationModel == "Exponential") return AttenuationModel::Exponential;

			VX_CORE_ASSERT(false, "Unknown Attenutation Model!");
			return AttenuationModel::None;
		}

		static std::string RigidBody2DBodyTypeToString(RigidBody2DType bodyType)
		{
			switch (bodyType)
			{
				case RigidBody2DType::Static:    return "Static";
				case RigidBody2DType::Dynamic:   return "Dynamic";
				case RigidBody2DType::Kinematic: return "Kinematic";
			}

			VX_CORE_ASSERT(false, "Unknown Body Type!");
			return {};
		}

		static RigidBody2DType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
		{
			if (bodyTypeString == "Static")    return RigidBody2DType::Static;
			if (bodyTypeString == "Dynamic")   return RigidBody2DType::Dynamic;
			if (bodyTypeString == "Kinematic") return RigidBody2DType::Kinematic;

			VX_CORE_ASSERT(false, "Unknown Body Type!");
			return RigidBody2DType::Static;
		}

		static std::string RigidBodyTypeToString(RigidBodyType bodyType)
		{
			switch (bodyType)
			{
				case RigidBodyType::Static:    return "Static";
				case RigidBodyType::Dynamic:   return "Dynamic";
			}

			VX_CORE_ASSERT(false, "Unknown Body Type!");
			return {};
		}

		static RigidBodyType RigidBodyTypeFromString(const std::string& bodyTypeString)
		{
			if (bodyTypeString == "Static")    return RigidBodyType::Static;
			if (bodyTypeString == "Dynamic")   return RigidBodyType::Dynamic;

			VX_CORE_ASSERT(false, "Unknown Body Type!");
			return RigidBodyType::Static;
		}

		static std::string CollisionDetectionTypeToString(CollisionDetectionType collisionDetection)
		{
			switch (collisionDetection)
			{
				case Vortex::CollisionDetectionType::Discrete:              return "Discrete";
				case Vortex::CollisionDetectionType::Continuous:            return "Continuous";
				case Vortex::CollisionDetectionType::ContinuousSpeculative: return "ContinuousSpeculative";
			}

			VX_CORE_ASSERT(false, "Unknown Collision Detection Type!");
			return {};
		}

		static CollisionDetectionType CollisionDetectionTypeFromString(const std::string& collisionDetectionString)
		{
			if (collisionDetectionString == "Discrete")              return CollisionDetectionType::Discrete;
			if (collisionDetectionString == "Continuous")            return CollisionDetectionType::Continuous;
			if (collisionDetectionString == "ContinuousSpeculative") return CollisionDetectionType::ContinuousSpeculative;

			VX_CORE_ASSERT(false, "Unknown Collision Detection Type!");
			return CollisionDetectionType::None;
		}

		static std::string NonWalkableModeToString(NonWalkableMode mode)
		{
			switch (mode)
			{
				case Vortex::NonWalkableMode::PreventClimbing:                return "PreventClimbing";
				case Vortex::NonWalkableMode::PreventClimbingAndForceSliding: return "PreventClimbingAndForceSliding";
			}

			VX_CORE_ASSERT(false, "Unknown Non Walkable Mode!");
			return "";
		}

		static NonWalkableMode NonWalkableModeFromString(const std::string& walkableMode)
		{
			if (walkableMode == "PreventClimbing") return NonWalkableMode::PreventClimbing;
			if (walkableMode == "PreventClimbingAndForceSliding") return NonWalkableMode::PreventClimbingAndForceSliding;

			VX_CORE_ASSERT(false, "Unknown Non Walkable Mode!");
			return NonWalkableMode::PreventClimbing;
		}

		static std::string CapsuleClimbModeToString(CapsuleClimbMode mode)
		{
			switch (mode)
			{
				case CapsuleClimbMode::Easy:        return "Easy";
				case CapsuleClimbMode::Constrained: return "Constrained";
			}

			VX_CORE_ASSERT(false, "Unknown Capsule Climb Mode!");
			return "";
		}

		static CapsuleClimbMode CapsuleClimbModeFromString(const std::string& climbMode)
		{
			if (climbMode == "Easy")        return CapsuleClimbMode::Easy;
			if (climbMode == "Constrained") return CapsuleClimbMode::Constrained;

			VX_CORE_ASSERT(false, "Unknown Capsule Climb Mode!");
			return CapsuleClimbMode::Easy;
		}

		static std::string CombineModeToString(CombineMode mode)
		{
			switch (mode)
			{
				case CombineMode::Average:  return "Average";
				case CombineMode::Max:      return "Max";
				case CombineMode::Min:      return "Min";
				case CombineMode::Mulitply: return "Multiply";
			}

			VX_CORE_ASSERT(false, "Unknown Combine Mode!");
			return "";
		}

		static CombineMode CombineModeFromString(const std::string& mode)
		{
			if (mode == "Average")  return CombineMode::Average;
			if (mode == "Max")     	return CombineMode::Average;
			if (mode == "Min")     	return CombineMode::Average;
			if (mode == "Multiply") return CombineMode::Average;

			VX_CORE_ASSERT(false, "Unknown Combine Mode!");
			return CombineMode::Average;
		}

	}

}

namespace YAML {

	template<>
	struct convert<Vortex::Math::vec2>
	{
		static Node encode(const Vortex::Math::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, Vortex::Math::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Vortex::Math::vec3>
	{
		static Node encode(const Vortex::Math::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, Vortex::Math::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Vortex::Math::vec4>
	{
		static Node encode(const Vortex::Math::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, Vortex::Math::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Vortex::Math::quaternion>
	{
		static Node encode(const Vortex::Math::quaternion& rhs)
		{
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, Vortex::Math::quaternion& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Vortex::UUID>
	{
		static Node encode(const Vortex::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Vortex::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};

	static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	static YAML::Emitter& operator<<(YAML::Emitter& out, const Vortex::Math::vec3& vector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vector.x << vector.y << vector.z << YAML::EndSeq;
		return out;
	}

	static YAML::Emitter& operator<<(YAML::Emitter& out, const Vortex::Math::vec4& vector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vector.x << vector.y << vector.z << vector.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

}
