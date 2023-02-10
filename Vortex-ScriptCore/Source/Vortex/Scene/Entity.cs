using System;

namespace Vortex {

	public class Entity
	{
		public readonly ulong ID;
		public Transform transform;

		public string Tag => InternalCalls.Entity_GetTag(ID);
		public string Marker => InternalCalls.Entity_GetMarker(ID);

		public Entity[] Children => InternalCalls.Entity_GetChildren(ID);

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

		protected virtual void OnCreate() { }
		protected virtual void OnUpdate(float delta) { }
		protected virtual void OnDestroy() { }
		protected virtual void OnCollisionBegin() { }
		protected virtual void OnCollisionEnd() { }
		protected virtual void OnTriggerBegin() { }
		protected virtual void OnTriggerEnd() { }
		protected virtual void OnRaycastCollision() { }
		protected virtual void OnGui() { }

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

		public Entity FindEntityByName(string name)
		{
			ulong entityID = InternalCalls.Entity_FindEntityByName(name);

			if (entityID == 0)
				return null;

			return new Entity(entityID);
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

		public T As<T>()
			where T : Entity, new()
		{
			object instance = InternalCalls.Entity_GetScriptInstance(ID);
			return instance as T;
		}

		public void Destroy(Entity entity, bool excludeChildren = false) => InternalCalls.Entity_Destroy(entity.ID, excludeChildren);

		public void SetActive(bool active)
		{
			InternalCalls.Entity_SetActive(ID, active);
		}
	}

}
