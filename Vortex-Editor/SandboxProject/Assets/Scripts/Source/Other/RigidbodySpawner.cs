using Vortex;

namespace Sandbox {

	public class RigidbodySpawner : Entity
	{
		bool m_Done = false;

		protected override void OnUpdate(float deltaTime)
		{
			if (Input.IsGamepadButtonDown(Gamepad.ButtonA) && !m_Done)
			{
				CreateRigidbodies();
				m_Done = true;
			}
		}

		void CreateRigidbodies()
		{
			uint i = 0;
			while (GetChild(i) != null)
			{
				Entity child = GetChild(i++);
				child.AddComponent<SphereCollider>();
				RigidBody rb = child.AddComponent<RigidBody>();
				rb.BodyType = RigidBodyType.Dynamic;
			}
		}
	}

}
