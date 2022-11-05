#include "sppch.h"
#include "SceneSerializer.h"

#include "Sparky/Scene/Entity.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Scripting/ScriptEngine.h"

#include "Sparky/Renderer/Model.h"
#include "Sparky/Renderer/LightSource.h"
#include "Sparky/Renderer/Skybox.h"
#include "Sparky/Renderer/ParticleEmitter.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
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
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
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
	struct convert<Sparky::UUID>
	{
		static Node encode(const Sparky::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Sparky::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};
}

namespace Sparky {

#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptField.GetValue<Type>();  \
				break;

#define READ_SCRIPT_FIELD(FieldType, Type)            \
	case ScriptFieldType::FieldType:                  \
	{                                                 \
		Type data = scriptField["Data"].as<Type>();   \
		fieldInstance.SetValue(data);                 \
		break;                                        \
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Math::vec3& vector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vector.x << vector.y << vector.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Math::vec4& vector)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vector.x << vector.y << vector.z << vector.w << YAML::EndSeq;
		return out;
	}

	static std::string LightTypeToString(LightSourceComponent::LightType lightType)
	{
		switch (lightType)
		{
			case LightSourceComponent::LightType::Directional:  return "Directional";
			case LightSourceComponent::LightType::Point:        return "Point";
			case LightSourceComponent::LightType::Spot:         return "Spot";
		}

		SP_CORE_ASSERT(false, "Unknown Light Type!");
		return {};
	}

	static LightSourceComponent::LightType LightTypeFromString(const std::string& lightTypeString)
	{
		if (lightTypeString == "Directional")  return LightSourceComponent::LightType::Directional;
		if (lightTypeString == "Point")        return LightSourceComponent::LightType::Point;
		if (lightTypeString == "Spot")         return LightSourceComponent::LightType::Spot;

		SP_CORE_ASSERT(false, "Unknown Light Type!");
		return LightSourceComponent::LightType::Directional;
	}

	static std::string MeshRendererMeshTypeToString(MeshRendererComponent::MeshType meshType)
	{
		switch (meshType)
		{
			case MeshRendererComponent::MeshType::Cube:     return "Cube";
			case MeshRendererComponent::MeshType::Sphere:   return "Sphere";
			case MeshRendererComponent::MeshType::Capsule:  return "Capsule";
			case MeshRendererComponent::MeshType::Cone:     return "Cone";
			case MeshRendererComponent::MeshType::Cylinder: return "Cylinder";
			case MeshRendererComponent::MeshType::Plane:    return "Plane";
			case MeshRendererComponent::MeshType::Torus:    return "Torus";
			case MeshRendererComponent::MeshType::Custom:   return "Custom";
		}

		SP_CORE_ASSERT(false, "Unknown Mesh Type!");
		return {};
	}

	static MeshRendererComponent::MeshType MeshRendererMeshTypeFromString(const std::string& meshTypeString)
	{
		if (meshTypeString == "Cube")     return MeshRendererComponent::MeshType::Cube;
		if (meshTypeString == "Sphere")   return MeshRendererComponent::MeshType::Sphere;
		if (meshTypeString == "Capsule")  return MeshRendererComponent::MeshType::Capsule;
		if (meshTypeString == "Cone")     return MeshRendererComponent::MeshType::Cone;
		if (meshTypeString == "Cylinder") return MeshRendererComponent::MeshType::Cylinder;
		if (meshTypeString == "Plane")    return MeshRendererComponent::MeshType::Plane;
		if (meshTypeString == "Torus")    return MeshRendererComponent::MeshType::Torus;
		if (meshTypeString == "Custom")   return MeshRendererComponent::MeshType::Custom;

		SP_CORE_ASSERT(false, "Unknown Mesh Type!");
		return MeshRendererComponent::MeshType::Cube;
	}

	static std::string RigidBody2DBodyTypeToString(RigidBody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case RigidBody2DComponent::BodyType::Static:    return "Static";
			case RigidBody2DComponent::BodyType::Dynamic:   return "Dynamic";
			case RigidBody2DComponent::BodyType::Kinematic: return "Kinematic";
		}

		SP_CORE_ASSERT(false, "Unknown Body Type!");
		return {};
	}

	static RigidBody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")    return RigidBody2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic")   return RigidBody2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return RigidBody2DComponent::BodyType::Kinematic;

		SP_CORE_ASSERT(false, "Unknown Body Type!");
		return RigidBody2DComponent::BodyType::Static;
	}

	SceneSerializer::SceneSerializer(const SharedRef<Scene>& scene)
		: m_Scene(scene) { }

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		SP_CORE_ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have a universally unique identifier!");

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent" << YAML::Value << YAML::BeginMap; // TagComponent
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}
		
		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap; // TransformComponent
			auto& transform = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SkyboxComponent>())
		{
			out << YAML::Key << "SkyboxComponent";
			out << YAML::BeginMap; // SkyboxComponent

			auto& lightComponent = entity.GetComponent<SkyboxComponent>();

			SharedRef<Skybox> skybox = lightComponent.Source;
			out << YAML::Key << "SourcePath" << skybox->GetDirectoryPath();

			out << YAML::EndMap; // SkyboxComponent
		}

		if (entity.HasComponent<LightSourceComponent>())
		{
			out << YAML::Key << "LightSourceComponent";
			out << YAML::BeginMap; // LightSourceComponent

			auto& lightComponent = entity.GetComponent<LightSourceComponent>();

			out << YAML::Key << "LightType" << YAML::Value << LightTypeToString(lightComponent.Type);
			
			SharedRef<LightSource> lightSource = lightComponent.Source;
			out << YAML::Key << "Ambient" << YAML::Value << lightSource->GetAmbient();
			out << YAML::Key << "Diffuse" << YAML::Value << lightSource->GetDiffuse();
			out << YAML::Key << "Specular" << YAML::Value << lightSource->GetSpecular();
			out << YAML::Key << "Color" << YAML::Value << lightSource->GetColor();

			switch (lightComponent.Type)
			{
				case LightSourceComponent::LightType::Directional:
				{
					out << YAML::Key << "Direction" << YAML::Value << lightSource->GetDirection();
					break;
				}
				case LightSourceComponent::LightType::Point:
				{
					out << YAML::Key << "Position" << YAML::Value << lightSource->GetPosition();
					out << YAML::Key << "Attenuation" << YAML::Value << lightSource->GetAttenuation();
					break;
				}
				case LightSourceComponent::LightType::Spot:
				{
					out << YAML::Key << "Position" << YAML::Value << lightSource->GetPosition();
					out << YAML::Key << "Direction" << YAML::Value << lightSource->GetDirection();
					out << YAML::Key << "Attenuation" << YAML::Value << lightSource->GetAttenuation();
					out << YAML::Key << "CutOff" << YAML::Value << lightSource->GetCutOff();
					out << YAML::Key << "OuterCutOff" << YAML::Value << lightSource->GetOuterCutOff();

					break;
				}
			}

			out << YAML::EndMap; // LightSourceComponent
		}

		if (entity.HasComponent<MeshRendererComponent>())
		{
			out << YAML::Key << "MeshRendererComponent" << YAML::Value << YAML::BeginMap; // MeshRendererComponent

			auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

			out << YAML::Key << "MeshType" << YAML::Value << MeshRendererMeshTypeToString(meshRendererComponent.Type);
			if (meshRendererComponent.Texture)
				out << YAML::Key << "TexturePath" << YAML::Value << meshRendererComponent.Texture->GetPath();
			if (meshRendererComponent.Mesh)
			{
				SharedRef<Model> model = meshRendererComponent.Mesh;
				out << YAML::Key << "MeshSource" << YAML::Value << model->GetPath();

				SharedRef<Material> material = model->GetMaterial();

				out << YAML::Key << "Ambient" << YAML::Value << material->GetAmbient();
				if (material->GetDiffuseMap())
					out << YAML::Key << "DiffuseMapPath" << YAML::Value << material->GetDiffuseMap()->GetPath();
				if (material->GetSpecularMap())
					out << YAML::Key << "SpecularMapPath" << YAML::Value << material->GetSpecularMap()->GetPath();

				out << YAML::Key << "Shininess" << material->GetShininess();
			}
			out << YAML::Key << "TextureScale" << YAML::Value << meshRendererComponent.Scale;
			out << YAML::Key << "Reflective" << YAML::Value << meshRendererComponent.Reflective;
			out << YAML::Key << "Refractive" << YAML::Value << meshRendererComponent.Refractive;

			out << YAML::EndMap; // MeshRendererComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent" << YAML::Value << YAML::BeginMap; // SpriteRendererComponent
			
			auto& spriteComponent = entity.GetComponent<SpriteRendererComponent>();

			out << YAML::Key << "Color" << YAML::Value << spriteComponent.SpriteColor;
			if (spriteComponent.Texture)
				out << YAML::Key << "TexturePath" << YAML::Value << spriteComponent.Texture->GetPath();
			out << YAML::Key << "TextureScale" << YAML::Value << spriteComponent.Scale;

			out << YAML::EndMap; // SpriteRendererComponent
		}
		
		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent" << YAML::Value << YAML::BeginMap; // CircleRendererComponent
			
			auto& circleComponent = entity.GetComponent<CircleRendererComponent>();

			out << YAML::Key << "Color" << YAML::Value << circleComponent.Color;
			out << YAML::Key << "Thickness" << YAML::Value << circleComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleComponent.Fade;

			out << YAML::EndMap; // CircleRendererComponent
		}

		if (entity.HasComponent<ParticleEmitterComponent>())
		{
			out << YAML::Key << "ParticleEmitterComponent" << YAML::Value << YAML::BeginMap; // ParticleEmitterComponent

			auto& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			SharedRef<ParticleEmitter> particleEmitter = particleEmitterComponent.Emitter;

			ParticleEmitterProperties emitterProperties = particleEmitter->GetProperties();

			out << YAML::Key << "ColorBegin" << YAML::Value << emitterProperties.ColorBegin;
			out << YAML::Key << "ColorEnd" << YAML::Value << emitterProperties.ColorEnd;
			out << YAML::Key << "LifeTime" << YAML::Value << emitterProperties.LifeTime;
			out << YAML::Key << "Position" << YAML::Value << emitterProperties.Position;
			out << YAML::Key << "Rotation" << YAML::Value << emitterProperties.Rotation;
			out << YAML::Key << "SizeBegin" << YAML::Value << emitterProperties.SizeBegin;
			out << YAML::Key << "SizeEnd" << YAML::Value << emitterProperties.SizeEnd;
			out << YAML::Key << "SizeVariation" << YAML::Value << emitterProperties.SizeVariation;
			out << YAML::Key << "Velocity" << YAML::Value << emitterProperties.Velocity;
			out << YAML::Key << "VelocityVariation" << YAML::Value << emitterProperties.VelocityVariation;

			out << YAML::EndMap; // ParticleEmitterComponent
		}

		if (entity.HasComponent<AudioSourceComponent>())
		{
			out << YAML::Key << "AudioSourceComponent" << YAML::Value << YAML::BeginMap; // AudioSourceComponent

			auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();

			if (audioSourceComponent.Source)
			{
				const AudioSource::SoundProperties& soundProperties = audioSourceComponent.Source->GetProperties();

				out << YAML::Key << "AudioSourcePath" << YAML::Value << audioSourceComponent.Source->GetPath();

				out << YAML::Key << "SoundSettings" << YAML::Value;
				out << YAML::BeginMap; // SoundSettings
				out << YAML::Key << "Position" << YAML::Value << soundProperties.Position;
				out << YAML::Key << "Direction" << YAML::Value << soundProperties.Direction;
				out << YAML::Key << "Veloctiy" << YAML::Value << soundProperties.Veloctiy;

				out << YAML::Key << "Cone" << YAML::Value;
				out << YAML::BeginMap; // Cone
				out << YAML::Key << "InnerAngle" << YAML::Value << soundProperties.Cone.InnerAngle;
				out << YAML::Key << "OuterAngle" << YAML::Value << soundProperties.Cone.OuterAngle;
				out << YAML::Key << "OuterGain" << YAML::Value << soundProperties.Cone.OuterGain;
				out << YAML::EndMap; // Cone

				out << YAML::Key << "MinDistance" << YAML::Value << soundProperties.MinDistance;
				out << YAML::Key << "MaxDistance" << YAML::Value << soundProperties.MaxDistance;
				out << YAML::Key << "Pitch" << YAML::Value << soundProperties.Pitch;
				out << YAML::Key << "DopplerFactor" << YAML::Value << soundProperties.DopplerFactor;
				out << YAML::Key << "Volume" << YAML::Value << soundProperties.Volume;

				out << YAML::Key << "Spacialized" << YAML::Value << soundProperties.Spacialized;
				out << YAML::Key << "Loop" << YAML::Value << soundProperties.Loop;
				out << YAML::EndMap; // SoundSettings
			}

			out << YAML::EndMap; // AudioSourceComponent
		}

		if (entity.HasComponent<RigidBody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent" << YAML::BeginMap; // Rigidbody2DComponent

			auto& rb2dComponent = entity.GetComponent<RigidBody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent" << YAML::BeginMap; // BoxCollider2DComponent

			auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}
		
		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent" << YAML::BeginMap; // CircleCollider2DComponent

			auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // CircleCollider2DComponent
		}

		// TODO: This may need reworking, specifically the random empty() check
		// if the script class name is empty don't even try to serialize, just move on
		if (entity.HasComponent<ScriptComponent>() && !entity.GetComponent<ScriptComponent>().ClassName.empty())
		{
			out << YAML::Key << "ScriptComponent" << YAML::BeginMap; // ScriptComponent

			auto& scriptComponent = entity.GetComponent<ScriptComponent>();
			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

			// Fields
			SharedRef<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();

			if (fields.size() > 0)
			{
				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);

				out << YAML::Key << "ScriptFields" << YAML::Value;
				out << YAML::BeginSeq;

				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					out << YAML::BeginMap; // ScriptFields

					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);
					out << YAML::Key << "Data" << YAML::Value;
					
					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float,   float)
						WRITE_SCRIPT_FIELD(Double,  double)
						WRITE_SCRIPT_FIELD(Bool,    bool)
						WRITE_SCRIPT_FIELD(Char,    int8_t)
						WRITE_SCRIPT_FIELD(Short,   int16_t)
						WRITE_SCRIPT_FIELD(Int,     int32_t)
						WRITE_SCRIPT_FIELD(Long,    int64_t)
						WRITE_SCRIPT_FIELD(Byte,    uint8_t)
						WRITE_SCRIPT_FIELD(UShort,  uint16_t)
						WRITE_SCRIPT_FIELD(UInt,    uint32_t)
						WRITE_SCRIPT_FIELD(ULong,   uint64_t)
						WRITE_SCRIPT_FIELD(Vector2, Math::vec2)
						WRITE_SCRIPT_FIELD(Vector3, Math::vec3)
						WRITE_SCRIPT_FIELD(Vector4, Math::vec4)
						WRITE_SCRIPT_FIELD(Entity,  UUID)
					}

					out << YAML::EndMap; // ScriptFields
				}

				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // ScriptComponent
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;

			SerializeEntity(out, entity);
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		SP_CORE_ASSERT(false, "Not Implemented Yet!");
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			SP_CORE_ERROR("Failed to load .sparky file '{}'\n     {}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		SP_CORE_TRACE("Deserializing Scene '{}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
				{
					name = tagComponent["Tag"].as<std::string>();
				}

				SP_CORE_TRACE("Deserialized Entity with ID = {}, name = {}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// All Entities have a transform
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<Math::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<Math::vec3>();
					tc.Scale = transformComponent["Scale"].as<Math::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					auto cameraProps = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Camera.SetPerspectiveVerticalFOV(Math::Deg2Rad(cameraProps["PerspectiveFOV"].as<float>()));
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto skyboxComponent = entity["SkyboxComponent"];
				if (skyboxComponent)
				{
					auto& skybox = deserializedEntity.AddComponent<SkyboxComponent>();

					skybox.Source = Skybox::Create(skyboxComponent["SourcePath"].as<std::string>());
				}

				auto lightSourceComponent = entity["LightSourceComponent"];
				if (lightSourceComponent)
				{
					auto& lightComponent = deserializedEntity.AddComponent<LightSourceComponent>();

					lightComponent.Source = CreateShared<LightSource>(LightSourceProperties());

					lightComponent.Type = LightTypeFromString(lightSourceComponent["LightType"].as<std::string>());
					if (lightSourceComponent["Ambient"])
						lightComponent.Source->SetAmbient(lightSourceComponent["Ambient"].as<Math::vec3>());
					if (lightSourceComponent["Diffuse"])
						lightComponent.Source->SetDiffuse(lightSourceComponent["Diffuse"].as<Math::vec3>());
					if (lightSourceComponent["Specular"])
						lightComponent.Source->SetSpecular(lightSourceComponent["Specular"].as<Math::vec3>());
					if (lightSourceComponent["Color"])
						lightComponent.Source->SetColor(lightSourceComponent["Color"].as<Math::vec3>());

					switch (lightComponent.Type)
					{
						case LightSourceComponent::LightType::Directional:
						{
							if (lightSourceComponent["Direction"])
								lightComponent.Source->SetDirection(lightSourceComponent["Direction"].as<Math::vec3>());

							break;
						}
						case LightSourceComponent::LightType::Point:
						{
							if (lightSourceComponent["Position"])
								lightComponent.Source->SetPosition(lightSourceComponent["Position"].as<Math::vec3>());
							if (lightSourceComponent["Attenuation"])
								lightComponent.Source->SetAttenuation(lightSourceComponent["Attenuation"].as<Math::vec2>());

							break;
						}
						case LightSourceComponent::LightType::Spot:
						{
							if (lightSourceComponent["Position"])
								lightComponent.Source->SetPosition(lightSourceComponent["Position"].as<Math::vec3>());
							if (lightSourceComponent["Direction"])
								lightComponent.Source->SetDirection(lightSourceComponent["Direction"].as<Math::vec3>());
							if (lightSourceComponent["Attenuation"])
								lightComponent.Source->SetAttenuation(lightSourceComponent["Attenuation"].as<Math::vec2>());
							if (lightSourceComponent["CutOff"])
								lightComponent.Source->SetCutOff(lightSourceComponent["CutOff"].as<float>());
							if (lightSourceComponent["OuterCutOff"])
								lightComponent.Source->SetOuterCutOff(lightSourceComponent["OuterCutOff"].as<float>());

							break;
						}
					}
				}

				auto meshComponent = entity["MeshRendererComponent"];
				if (meshComponent)
				{
					auto& meshRendererComponent = deserializedEntity.AddComponent<MeshRendererComponent>();

					meshRendererComponent.Type = MeshRendererMeshTypeFromString(meshComponent["MeshType"].as<std::string>());

					if (meshComponent["TexturePath"])
						meshRendererComponent.Texture = Texture2D::Create(meshComponent["TexturePath"].as<std::string>());
					if (meshComponent["MeshSource"])
						meshRendererComponent.Mesh = Model::Create(meshComponent["MeshSource"].as<std::string>(), deserializedEntity);
					if (meshComponent["Ambient"])
						meshRendererComponent.Mesh->GetMaterial()->SetAmbient(meshComponent["Ambient"].as<Math::vec3>());
					if (meshComponent["DiffuseMapPath"])
						meshRendererComponent.Mesh->GetMaterial()->SetDiffuseMap(Texture2D::Create(meshComponent["DiffuseMapPath"].as<std::string>()));
					if (meshComponent["SpecularMapPath"])
						meshRendererComponent.Mesh->GetMaterial()->SetSpecularMap(Texture2D::Create(meshComponent["SpecularMapPath"].as<std::string>()));
					if (meshComponent["Shininess"])
						meshRendererComponent.Mesh->GetMaterial()->SetShininess(meshComponent["Shininess"].as<float>());

					if (meshComponent["TextureScale"])
						meshRendererComponent.Scale = meshComponent["TextureScale"].as<Math::vec2>();

					if (meshComponent["Reflective"])
						meshRendererComponent.Reflective = meshComponent["Reflective"].as<bool>();
					if (meshComponent["Refractive"])
						meshRendererComponent.Refractive= meshComponent["Refractive"].as<bool>();
				}

				auto spriteComponent = entity["SpriteRendererComponent"];
				if (spriteComponent)
				{
					auto& spriteRendererComponent = deserializedEntity.AddComponent<SpriteRendererComponent>();

					spriteRendererComponent.SpriteColor = spriteComponent["Color"].as<Math::vec4>();

					if (spriteComponent["TexturePath"])
						spriteRendererComponent.Texture = Texture2D::Create(spriteComponent["TexturePath"].as<std::string>());

					if (spriteComponent["TextureScale"])
						spriteRendererComponent.Scale = spriteComponent["TextureScale"].as<Math::vec2>();
				}
				
				auto circleComponent = entity["CircleRendererComponent"];
				if (circleComponent)
				{
					auto& circleRendererComponent = deserializedEntity.AddComponent<CircleRendererComponent>();

					circleRendererComponent.Color = circleComponent["Color"].as<Math::vec4>();
					circleRendererComponent.Thickness = circleComponent["Thickness"].as<float>();
					circleRendererComponent.Fade = circleComponent["Fade"].as<float>();
				}

				auto particleEmitterComponent = entity["ParticleEmitterComponent"];
				if (particleEmitterComponent)
				{
					auto& particleEmitter = deserializedEntity.AddComponent<ParticleEmitterComponent>();

					ParticleEmitterProperties emitterProperties;
					emitterProperties.ColorBegin = particleEmitterComponent["ColorBegin"].as<Math::vec4>();
					emitterProperties.ColorEnd = particleEmitterComponent["ColorEnd"].as<Math::vec4>();
					emitterProperties.LifeTime = particleEmitterComponent["LifeTime"].as<float>();
					emitterProperties.Position = particleEmitterComponent["Position"].as<Math::vec3>();
					emitterProperties.Rotation = particleEmitterComponent["Rotation"].as<float>();
					emitterProperties.SizeBegin = particleEmitterComponent["SizeBegin"].as<Math::vec2>();
					emitterProperties.SizeEnd = particleEmitterComponent["SizeEnd"].as<Math::vec2>();
					emitterProperties.SizeVariation = particleEmitterComponent["SizeVariation"].as<Math::vec2>();
					emitterProperties.Velocity = particleEmitterComponent["Velocity"].as<Math::vec3>();
					emitterProperties.VelocityVariation = particleEmitterComponent["VelocityVariation"].as<Math::vec3>();

					particleEmitter.Emitter = ParticleEmitter::Create(emitterProperties);
				}

				auto audioSourceComponent = entity["AudioSourceComponent"];
				if (audioSourceComponent)
				{
					auto& asc = deserializedEntity.AddComponent<AudioSourceComponent>();

					if (audioSourceComponent["AudioSourcePath"])
						asc.Source = CreateShared<AudioSource>(audioSourceComponent["AudioSourcePath"].as<std::string>());

					auto soundProps = audioSourceComponent["SoundSettings"];

					if (soundProps)
					{
						auto& soundProperties = asc.Source->GetProperties();
						if (soundProps["Position"])
							soundProperties.Position = soundProps["Position"].as<Math::vec3>();
						if (soundProps["Direction"])
							soundProperties.Direction = soundProps["Direction"].as<Math::vec3>();
						if (soundProps["Velocity"])
							soundProperties.Veloctiy = soundProps["Velocity"].as<Math::vec3>();

						if (soundProps["Cone"])
						{
							auto cone = soundProps["Cone"];
							soundProperties.Cone.InnerAngle = cone["InnerAngle"].as<float>();
							soundProperties.Cone.OuterAngle = cone["OuterAngle"].as<float>();
							soundProperties.Cone.OuterGain = cone["OuterGain"].as<float>();
						}

						if (soundProps["MinDistance"])
							soundProperties.MinDistance = soundProps["MinDistance"].as<float>();
						if (soundProps["MaxDistance"])
							soundProperties.MaxDistance = soundProps["MaxDistance"].as<float>();
						
						if (soundProps["Pitch"])
							soundProperties.Pitch = soundProps["Pitch"].as<float>();
						if (soundProps["DopplerFactor"])
							soundProperties.DopplerFactor = soundProps["DopplerFactor"].as<float>();
						if (soundProps["Volume"])
							soundProperties.Volume = soundProps["Volume"].as<float>();

						if (soundProps["Spacialized"])
							soundProperties.Spacialized = soundProps["Spacialized"].as<bool>();
						if (soundProps["Loop"])
							soundProperties.Loop = soundProps["Loop"].as<bool>();
					}
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					auto& rb2d = deserializedEntity.AddComponent<RigidBody2DComponent>();

					rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();

					bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
					bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
					bc2d.Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();

					cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
					cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
					cc2d.Density = circleCollider2DComponent["Density"].as<float>();
					cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
					cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
					cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
					sc.ClassName = scriptComponent["ClassName"].as<std::string>();

					auto scriptFields = scriptComponent["ScriptFields"];
					if (scriptFields)
					{
						SharedRef<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
						SP_CORE_ASSERT(entityClass, "Entity class was null pointer!");
						const auto& fields = entityClass->GetFields();
						auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

						for (auto scriptField : scriptFields)
						{
							std::string name = scriptField["Name"].as<std::string>();
							std::string typeString = scriptField["Type"].as<std::string>();
							ScriptFieldType type = Utils::StringToScriptFieldType(typeString);

							ScriptFieldInstance& fieldInstance = entityFields[name];
							
							// TODO: Turn this into editor log warning
							SP_CORE_ASSERT(fields.find(name) != fields.end(), "Script Field was not found in Field Map!");
							if (fields.find(name) == fields.end())
								continue;

							fieldInstance.Field = fields.at(name);

							switch (type)
							{
								READ_SCRIPT_FIELD(Float,   float)
								READ_SCRIPT_FIELD(Double,  double)
								READ_SCRIPT_FIELD(Bool,    bool)
								READ_SCRIPT_FIELD(Char,    int8_t)
								READ_SCRIPT_FIELD(Short,   int16_t)
								READ_SCRIPT_FIELD(Int,     int32_t)
								READ_SCRIPT_FIELD(Long,    int64_t)
								READ_SCRIPT_FIELD(Byte,    uint8_t)
								READ_SCRIPT_FIELD(UShort,  uint16_t)
								READ_SCRIPT_FIELD(UInt,    uint32_t)
								READ_SCRIPT_FIELD(ULong,   uint64_t)
								READ_SCRIPT_FIELD(Vector2, Math::vec2)
								READ_SCRIPT_FIELD(Vector3, Math::vec3)
								READ_SCRIPT_FIELD(Vector4, Math::vec4)
								READ_SCRIPT_FIELD(Entity,  UUID)
							}
						}
					}
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		SP_CORE_ASSERT(false, "Not Implemented Yet!");
		return false;
	}

}
