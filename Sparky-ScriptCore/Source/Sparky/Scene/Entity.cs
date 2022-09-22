using System;

namespace Sparky {

	public class Entity
	{
        protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
            ID = id;
			Transform = GetComponent<Transform>();
		}

        public readonly ulong ID;
		public Transform Transform;

		public Vector3 Translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translation);
				return translation;
			}
			
			set
			{
				InternalCalls.TransformComponent_SetTranslation(ID, ref value);
			}
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
	}

}