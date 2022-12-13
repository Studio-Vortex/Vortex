using Vortex;

namespace Sandbox {

	public class CharCont : Entity
	{
		CharacterController cont;
		protected override void OnCreate()
		{
			cont = GetComponent<CharacterController>();
		}

		protected override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.W))
				cont.Move(Vector3.Forward * delta);
			else if (Input.IsKeyDown(KeyCode.S))
				cont.Move(Vector3.Back * delta);

			if (Input.IsKeyDown(KeyCode.A))
				cont.Move(Vector3.Left * delta);
			else if (Input.IsKeyDown(KeyCode.D))
				cont.Move(Vector3.Right * delta);

			if (cont.IsGrounded && Input.IsKeyDown(KeyCode.Space))
				cont.Jump(5f);
		}
	}

}
