namespace Vortex {

	public class Collider
	{
		public ulong EntityID { get; protected set; }
		public bool IsTrigger { get; protected set; }

		private Entity m_Entity;
		private RigidBody m_RigidBody;

		public Entity Entity
		{
			get
			{
				if (m_Entity == null)
					m_Entity = new Entity(EntityID);

				return m_Entity;
			}
		}

		public RigidBody RigidBody
		{
			get => m_RigidBody ?? (m_RigidBody = Entity.GetComponent<RigidBody>());
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

			return $"Collider({type}, {EntityID}, {IsTrigger})";
		}
	}

	public class HitBoxCollider: Collider
	{
		public Vector3 HalfSize { get; protected set; }
		public Vector3 Offset { get; protected set; }

		internal HitBoxCollider()
		{
			EntityID = 0;
			IsTrigger = false;
			HalfSize = Vector3.Zero;
			Offset = Vector3.Zero;
		}

		internal HitBoxCollider(ulong entityID, bool isTrigger, Vector3 halfSize, Vector3 offset)
		{
			EntityID = entityID;
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
			EntityID = 0;
			IsTrigger = false;
			Radius = 0.0f;
		}

		internal HitSphereCollider(ulong entityID, bool isTrigger, float radius)
		{
			EntityID = entityID;
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
			EntityID = 0;
			IsTrigger = false;
			Radius = 0.0f;
			Height = 0.0f;
		}

		internal HitCapsuleCollider(ulong entityID, bool isTrigger, float radius, float height)
		{
			EntityID = entityID;
			IsTrigger = isTrigger;
			Radius = radius;
			Height = height;
		}
	}

}
