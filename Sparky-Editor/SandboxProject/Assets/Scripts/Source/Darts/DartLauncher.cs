using System.Collections.Generic;
using Sparky;

namespace Sandbox {

	public class DartLauncher : Entity
	{
		List<Entity> m_Darts;

		protected override void OnCreate()
		{
			m_Darts = new List<Entity>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			if (Time.Elapsed % 3 <= 0.01f)
				SpawnDart();

			foreach (Entity dart in m_Darts)
			{
				Vector2 end = dart.transform.Translation.XY + (Vector2.Right * 0.52f);
				Entity other = Physics2D.Raycast(dart.transform.Translation.XY, end, out RayCastHit2D hitInfo);

				if (hitInfo.Hit && other.Tag == "Circle")
				{
					dart.transform.SetParent(other);
					dart.RemoveComponent<RigidBody2D>();
					dart.RemoveComponent<BoxCollider2D>();
					m_Darts.Remove(dart);
				}
			}
		}

		void SpawnDart()
		{
			Entity dart = new Entity("Dart");
			dart.transform.Translation = new Vector3(-4, Vector2.Zero);
			dart.transform.Scale = new Vector3(1, 0.1f, 1);
			dart.AddComponent<SpriteRenderer>();
			dart.AddComponent<BoxCollider2D>();
			RigidBody2D rb = dart.AddComponent<RigidBody2D>();
			rb.BodyType = RigidBody2DType.Dynamic;
			rb.Velocity = Vector2.Right;
			m_Darts.Add(dart);
		}
	}

}
