using System;

namespace Vortex {

	public class Entity
	{
		public readonly ulong ID;
		public Transform transform;

		public string Tag => InternalCalls.Entity_GetTag(ID);
		public string Marker
		{
			get => InternalCalls.Entity_GetMarker(ID);
			set => InternalCalls.Entity_SetMarker(ID, value);
		}

		public Entity[] Children => InternalCalls.Entity_GetChildren(ID);

		protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
			ID = id;
			transform = GetComponent<Transform>();
		}

		protected virtual void OnAwake() { }
		protected virtual void OnCreate() { }
		protected virtual void OnUpdate(float delta) { }
		protected virtual void OnUpdate() { }
		protected virtual void OnDestroy() { }
		protected virtual void OnCollisionEnter(Collision other) { }
		protected virtual void OnCollisionExit(Collision other) { }
		protected virtual void OnTriggerEnter(Collision other) { }
		protected virtual void OnTriggerExit(Collision other) { }
		protected virtual void OnFixedJointDisconnected(Vector3 linearForce, Vector3 angularForce) { }
		protected virtual void OnRaycastCollision() { }
		protected virtual void OnEnabled() { }
		protected virtual void OnDisabled() { }
		protected virtual void OnGui() { }

		public void Destroy(Entity entity, bool excludeChildren = false) => InternalCalls.Entity_Destroy(entity.ID, excludeChildren);

		public Entity FindEntityByName(string name) => Scene.FindEntityByName(name);

		public Entity FindChildByName(string name)
		{
			ulong entityID = InternalCalls.Entity_FindChildByName(ID, name);

			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

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
				Type componentType = typeof(T);
				InternalCalls.Entity_AddComponent(ID, componentType);
				return GetComponent<T>();
			}
			else
			{
				return GetComponent<T>();
			}
		}

		public void RemoveComponent<T>()
			where T : Component, new()
		{
			if (HasComponent<T>())
			{
				Type componentType = typeof(T);
				InternalCalls.Entity_RemoveComponent(ID, componentType);
			}
		}

		public bool TryGetComponent<T>(out T component)
			where T : Component, new()
		{
			if (!HasComponent<T>())
			{
				component = null;
				return false;
			}

			component = new T() { Entity = this };
			return true;
		}

		public Entity GetChild(uint index)
		{
			ulong entityID = InternalCalls.Entity_GetChild(ID, index);

			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public bool AddChild(Entity child) =>  InternalCalls.Entity_AddChild(ID, child.ID);

		public bool RemoveChild(Entity child) => InternalCalls.Entity_RemoveChild(ID, child.ID);

		public bool Is<T>()
			where T : Entity, new()
		{
			object instance = InternalCalls.Entity_GetScriptInstance(ID);
			return instance is T;
		}

		public T As<T>()
			where T : Entity, new()
		{
			object instance = InternalCalls.Entity_GetScriptInstance(ID);
			return instance as T;
		}

		public void SetActive(bool active) => InternalCalls.Entity_SetActive(ID, active);

		public override bool Equals(object obj) => obj is Entity other && Equals(other);

		public bool Equals(Entity other)
		{
			if (other is null)
				return false;

			if (ReferenceEquals(this, other))
				return true;

			return ID == other.ID;
		}

		public override int GetHashCode() => (int)ID;

		public static bool operator ==(Entity entityA, Entity entityB) => entityA is null ? entityB is null : entityA.Equals(entityB);
		public static bool operator !=(Entity entityA, Entity entityB) => !(entityA == entityB);
	}

}
