using System;

namespace Sparky {

	public class Entity
	{
		public readonly ulong ID;
		public Transform transform;

		protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
			ID = id;
			transform = GetComponent<Transform>();
		}

		public virtual void OnCreate() { }
		public virtual void OnUpdate(float delta) { }
		public virtual void OnDestroy() { }

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

		public void Destroy()
		{
			InternalCalls.Entity_Destroy(ID);
		}
	}

}