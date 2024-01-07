using System.Runtime.CompilerServices;
using System;

namespace Vortex {

	public class Actor
	{
		public readonly ulong ID;
		public Transform transform;

		public string Tag
		{
			get => InternalCalls.Actor_GetTag(ID);
			set => InternalCalls.Actor_SetTag(ID, value);
		}

		public string Marker
		{
			get => InternalCalls.Actor_GetMarker(ID);
			set => InternalCalls.Actor_SetMarker(ID, value);
		}

		public Timer AddTimer(float delay, [CallerLineNumber] int lineNumber = 0)
		{
			string name = Tag + lineNumber;
			return AddTimer(name, delay);
		}

		public Timer AddTimer(string name, float delay)
		{
			InternalCalls.Actor_AddTimer(ID, name, delay);
			return new Timer(ID, name);
		}

		public Timer GetTimer(string name)
		{
			return new Timer(ID, name);
		}

		public Actor[] Children => InternalCalls.Actor_GetChildren(ID);

		public bool ActiveInHierarchy => InternalCalls.Actor_IsActive(ID);
		public void SetActive(bool active) => InternalCalls.Actor_SetActive(ID, active);

		protected Actor() { ID = 0; }

		internal Actor(ulong id)
		{
			ID = id;
			transform = GetComponent<Transform>();
		}

		public Actor(string name = "")
		{
			ID = InternalCalls.Scene_CreateActor(name);
			transform = GetComponent<Transform>();
		}

		protected virtual void OnAwake() { }
		protected virtual void OnEnable() { }
		protected virtual void OnCreate() { }
		protected virtual void OnUpdate() { }
		protected virtual void OnPostUpdate() { }
		protected virtual void OnDebugRender() { }
		protected virtual void OnCollisionEnter(Collision other) { }
		protected virtual void OnCollisionExit(Collision other) { }
		protected virtual void OnTriggerEnter(Collision other) { }
		protected virtual void OnTriggerExit(Collision other) { }
		protected virtual void OnFixedJointDisconnected(Vector3 linearForce, Vector3 angularForce) { }
		protected virtual void OnApplicationQuit() { }
		protected virtual void OnDisable() { }
		protected virtual void OnDestroy() { }
		protected virtual void OnGuiRender() { }

		public Actor GetChild(uint index)
		{
			ulong actorID = InternalCalls.Actor_GetChild(ID, index);

			if (actorID == 0)
				return null;

			return new Actor(actorID);
		}

		public bool AddChild(Actor child) => InternalCalls.Actor_AddChild(ID, child.ID);
		public bool RemoveChild(Actor child) => InternalCalls.Actor_RemoveChild(ID, child.ID);

		public Actor FindActorByName(string name) => Scene.FindActorByName(name);
		public Actor FindChildByName(string name) => Scene.FindChildByName(this, name);

		public Actor Instantiate(Actor actor) => Scene.Instantiate(actor);
		public Actor Instantiate(Actor actor, Vector3 worldPos) => Scene.Instantiate(actor, worldPos);
		public Actor Instantiate(Actor actor, Vector3 worldPos, Quaternion orientation) => Scene.Instantiate(actor, worldPos, orientation);
		public Actor Instantiate(Actor actor, Actor parent) => Scene.Instantiate(actor, parent);
		public Actor Instantiate(Actor actor, Actor parent, Vector3 worldPos) => Scene.Instantiate(actor, parent, worldPos);
		public Actor Instantiate(Actor actor, Actor parent, Vector3 worldPos, Quaternion orientation) => Scene.Instantiate(actor, parent, worldPos, orientation);

		public void Invoke(string method) => InternalCalls.Actor_Invoke(ID, method);
		public void Invoke(string method, float delay) => InternalCalls.Actor_InvokeWithDelay(ID, method, delay);

		public void Destroy(Actor actor, bool excludeChildren = false) => InternalCalls.Actor_Destroy(actor.ID, excludeChildren);
		public void Destroy(Actor actor, float delay, bool excludeChildren = false) => InternalCalls.Actor_DestroyWithDelay(actor.ID, delay, excludeChildren);

		public bool HasComponent<T>()
			where T : Component, new()
		{
			Type componentType = typeof(T);
			return InternalCalls.Actor_HasComponent(ID, componentType);
		}

		public T GetComponent<T>()
			where T : Component, new()
		{
			if (!HasComponent<T>())
				return null;

			T component = new T() { Actor = this };
			return component;
		}

		public T AddComponent<T>()
			where T : Component, new()
		{
			if (!HasComponent<T>())
			{
				Type componentType = typeof(T);
				InternalCalls.Actor_AddComponent(ID, componentType);
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
				InternalCalls.Actor_RemoveComponent(ID, componentType);
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

			component = new T() { Actor = this };
			return true;
		}

		public bool Is<T>()
			where T : Actor, new()
		{
			object instance = InternalCalls.Actor_GetScriptInstance(ID);
			return instance is T;
		}

		public T As<T>()
			where T : Actor, new()
		{
			object instance = InternalCalls.Actor_GetScriptInstance(ID);
			return instance as T;
		}

		public bool CompareTag(string tag)
		{
			if (Tag.Length != tag.Length) {
				return false;
			}

			return Tag.Equals(tag);
		}

		public override bool Equals(object obj) => obj is Actor other && Equals(other);

		public bool Equals(Actor other)
		{
			if (other is null)
				return false;

			if (ReferenceEquals(this, other))
				return true;

			return ID == other.ID;
		}

		public override int GetHashCode() => (int)ID;

		public static bool operator ==(Actor actorA, Actor actorB) => !actorA ? !actorB : actorA.Equals(actorB);
		public static bool operator !=(Actor actorA, Actor actorB) => !(actorA == actorB);
		public static implicit operator bool(Actor actor) => InternalCalls.Actor_IsValid(actor.ID);
	}

}
