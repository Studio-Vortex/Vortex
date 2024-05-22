namespace Vortex {

	public class Collider
	{
		public ulong ActorID { get; protected set; }
		public bool IsTrigger { get; protected set; }

		private Actor m_Actor;
		private RigidBody m_RigidBody;

		public Actor Actor
		{
			get
			{
				if (m_Actor == null)
					m_Actor = new Actor(ActorID);

				return m_Actor;
			}
		}

		public RigidBody RigidBody
		{
			get => m_RigidBody ?? (m_RigidBody = Actor.GetComponent<RigidBody>());
		}

		public override string ToString()
		{
			string type = "Collider";
			
			switch (this)
			{
				case HitBoxCollider _:
					type = "BoxCollider";
					break;
				case HitSphereCollider _:
					type = "SphereCollider";
					break;
				case HitCapsuleCollider _:
					type = "CapsuleCollider";
					break;
			}

			return $"Collider({type}, {ActorID}, {IsTrigger})";
		}
	}

	public class HitBoxCollider: Collider
	{
		public Vector3 HalfSize { get; protected set; }
		public Vector3 Offset { get; protected set; }

		internal HitBoxCollider()
		{
			ActorID = 0;
			IsTrigger = false;
			HalfSize = Vector3.Zero;
			Offset = Vector3.Zero;
		}

		internal HitBoxCollider(ulong actorID, bool isTrigger, Vector3 halfSize, Vector3 offset)
		{
			ActorID = actorID;
			IsTrigger = isTrigger;
			HalfSize = halfSize;
			Offset = offset;
		}
	}

	public class HitSphereCollider : Collider
	{
		public float Radius { get; protected set; }

		internal HitSphereCollider()
		{
			ActorID = 0;
			IsTrigger = false;
			Radius = 0.0f;
		}

		internal HitSphereCollider(ulong actorID, bool isTrigger, float radius)
		{
			ActorID = actorID;
			IsTrigger = isTrigger;
			Radius = radius;
		}
	}

	public class HitCapsuleCollider : Collider
	{
		public float Radius { get; protected set; }
		public float Height { get; protected set; }

		internal HitCapsuleCollider()
		{
			ActorID = 0;
			IsTrigger = false;
			Radius = 0.0f;
			Height = 0.0f;
		}

		internal HitCapsuleCollider(ulong actorID, bool isTrigger, float radius, float height)
		{
			ActorID = actorID;
			IsTrigger = isTrigger;
			Radius = radius;
			Height = height;
		}
	}

}
