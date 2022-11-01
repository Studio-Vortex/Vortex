using System;

namespace Sparky {

	public class Entity
	{
		public readonly ulong ID;
		public Transform transform;
		public string Tag
		{
			get
			{
				InternalCalls.Entity_GetName(ID, out string name);
				return name;
			}
		}

		protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
			ID = id;
			transform = GetComponent<Transform>();
		}

		public Entity(string name)
		{
			ID = InternalCalls.Entity_CreateWithName(name);
			transform = GetComponent<Transform>();
		}

		public virtual void OnCreate() { }
		public virtual void OnUpdate(float delta) { }
		public virtual void OnDestroy() { }
		public virtual void OnCollision() { }
		public virtual void OnGui() { }

		public bool HasComponent<T>()
			where T : Component, new()
		{
			Type componentType = typeof(T);
			return InternalCalls.Entity_HasComponent(ID, componentType);
		}

		public T GetComponent<T>()
			where T : Component, new()
		{
			if (!HasComponent<T>())
				return null;

			T component = new T() { Entity = this };
			return component;
		}

		public T AddComponent<T>()
			where T : Component, new()
		{
			if (!HasComponent<T>())
			{
				string componentName = typeof(T).ToString();

				switch (componentName)
				{
					case "Sparky.Camera":           InternalCalls.Entity_AddCamera(ID);           break;
					case "Sparky.LightSource":      InternalCalls.Entity_AddLightSource(ID);      break;
					case "Sparky.MeshRenderer":     InternalCalls.Entity_AddMeshRenderer(ID);     break;
					case "Sparky.SpriteRenderer":   InternalCalls.Entity_AddSpriteRenderer(ID);   break;
					case "Sparky.CircleRenderer":   InternalCalls.Entity_AddCircleRenderer(ID);   break;
					case "Sparky.ParticleEmitter":  InternalCalls.Entity_AddParticleEmitter(ID);  break;
					case "Sparky.AudioSource":      InternalCalls.Entity_AddAudioSource(ID);      break;
					case "Sparky.AudioListener":    InternalCalls.Entity_AddAudioListener(ID);    break;
					case "Sparky.RigidBody2D":      InternalCalls.Entity_AddRigidBody2D(ID);      break;
					case "Sparky.BoxCollider2D":    InternalCalls.Entity_AddBoxCollider2D(ID);    break;
					case "Sparky.CircleCollider2D": InternalCalls.Entity_AddCircleCollider2D(ID); break;
				}

				return GetComponent<T>();
			}

			return null;
		}

		public void RemoveComponent<T>()
			where T : Component, new()
		{
			if (HasComponent<T>())
			{
				string componentName = typeof(T).ToString();

				switch (componentName)
				{
					case "Sparky.Camera":           InternalCalls.Entity_RemoveCamera(ID);           break;
					case "Sparky.LightSource":      InternalCalls.Entity_RemoveLightSource(ID);      break;
					case "Sparky.MeshRenderer":     InternalCalls.Entity_RemoveMeshRenderer(ID);     break;
					case "Sparky.SpriteRenderer":   InternalCalls.Entity_RemoveSpriteRenderer(ID);   break;
					case "Sparky.CircleRenderer":   InternalCalls.Entity_RemoveCircleRenderer(ID);   break;
					case "Sparky.ParticleEmitter":  InternalCalls.Entity_RemoveParticleEmitter(ID);  break;
					case "Sparky.AudioSource":      InternalCalls.Entity_RemoveAudioSource(ID);      break;
					case "Sparky.AudioListener":    InternalCalls.Entity_RemoveAudioListener(ID);    break;
					case "Sparky.RigidBody2D":      InternalCalls.Entity_RemoveRigidBody2D(ID);      break;
					case "Sparky.BoxCollider2D":    InternalCalls.Entity_RemoveBoxCollider2D(ID);    break;
					case "Sparky.CircleCollider2D": InternalCalls.Entity_RemoveCircleCollider2D(ID); break;
				}
			}
		}

		public Entity FindEntityByName(string name)
		{
			ulong entityID = InternalCalls.Entity_FindEntityByName(name);

			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public T As<T>()
			where T : Entity, new()
		{
			object instance = InternalCalls.Entity_GetScriptInstance(ID);
			return instance as T;
		}

		public void Destroy(bool scriptInstance = true)
		{
			InternalCalls.Entity_Destroy(ID, isScriptInstance: scriptInstance);
		}
	}

}
