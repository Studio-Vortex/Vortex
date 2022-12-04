using Sparky;

namespace Sandbox {

	public class Dart : Entity
	{
		RigidBody2D rb;
		bool done = false;

		protected override void OnCreate()
		{
			rb = GetComponent<RigidBody2D>();
			rb.Velocity = Vector2.Right;
		}
		
		protected override void OnUpdate(float deltaTime)
		{
			if (done)
				return;

			Vector2 end = transform.Translation.XY + (Vector2.Right * 0.52f);
			Entity other = Physics2D.Raycast(transform.Translation.XY, end, out RayCastHit2D hitInfo);

			if (hitInfo.Hit && other.Tag == "Circle")
			{
				transform.SetParent(other);
				RemoveComponent<RigidBody2D>();
				RemoveComponent<BoxCollider2D>();
				done = true;
			}
		}
	}

}
