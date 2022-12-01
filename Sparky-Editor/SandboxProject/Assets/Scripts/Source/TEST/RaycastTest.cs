using Sparky;

namespace Sandbox {

	public class RaycastTest : Entity
	{
		Entity camera;
		Entity ball;
		RigidBody rigidbody;

		protected override void OnCreate()
		{
			ball = new Entity("Ball");
			MeshRenderer meshRenderer = ball.AddComponent<MeshRenderer>();
			meshRenderer.Type = MeshType.Cube;
			Material material = meshRenderer.GetMaterial();
			material.Albedo = Color.Red.XYZ;
			ball.transform.Translation = Vector3.Up * 2;

			ball.AddComponent<BoxCollider>();
			rigidbody = ball.AddComponent<RigidBody>();
			rigidbody.BodyType = RigidBodyType.Dynamic;
			rigidbody.DisableGravity = true;

			camera = FindEntityByName("Camera");
		}

		protected override void OnUpdate(float deltaTime)
		{
			Vector3 speed = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
			{
				speed = Vector3.Forward;
			}
			else if (Input.IsKeyDown(KeyCode.S))
			{
				speed = Vector3.Back;
			}

			if (Input.IsKeyDown(KeyCode.A))
			{
				speed = Vector3.Left;
			}
			else if (Input.IsKeyDown(KeyCode.D))
			{
				speed = Vector3.Right;
			}

			rigidbody.AddForce(speed);
			rigidbody.AddTorque(speed);

			camera.transform.Translation = ball.transform.Translation + (Vector3.Up * 3) + (Vector3.Back * 7);
		}
	}

}
