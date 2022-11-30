using Sparky;

namespace Sandbox {

	public class CubeSpawner : Entity
	{
		public float frequency = 3f;
		public float baseImpulse = 50f;

		protected override void OnUpdate(float deltaTime)
		{
			if (Time.Elapsed % frequency <= 0.01f)
			{
				CreateCube();
			}
		}

		void CreateCube()
		{
			Entity cube = new Entity("Cube");
			cube.transform.Translation = transform.Translation;
			MeshRenderer meshRenderer = cube.AddComponent<MeshRenderer>();
			Material material = meshRenderer.GetMaterial();
			material.Albedo = new Vector3(RandomDevice.RangedFloat(0, 1), RandomDevice.RangedFloat(0, 1), RandomDevice.RangedFloat(0, 1));
			cube.AddComponent<BoxCollider>();
			RigidBody rigidbody = cube.AddComponent<RigidBody>();
			rigidbody.BodyType = RigidBodyType.Dynamic;
			Vector3 random = new Vector3(RandomDevice.RangedFloat(1, 2), RandomDevice.RangedFloat(1, 2), RandomDevice.RangedFloat(1, 2));
			rigidbody.AddForce(Vector3.Up * baseImpulse * random, ForceMode.Impulse);
		}
	}

}
