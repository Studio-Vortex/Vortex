using System;
using Sparky;

namespace Sandbox {

	public class Rotate : Entity
	{
		public float Speed;
		public bool Created = false;
		public Entity entity;

		public override void OnUpdate(float delta)
		{
			float speed = Speed * delta;

			transform.Rotate(speed, speed, speed);
			
			if (!Created && Input.IsKeyDown(KeyCode.Space))
			{
				entity = new Entity("Mesh Entity");
				entity.transform.Translate(-5.0f, 0.0f, -3.0f);
				entity.AddComponent<MeshRenderer>().Texture = "Assets/Textures/Other/wood.jpg";
				Created = true;
			}

			if (Created)
				entity.transform.Rotate(speed, speed, speed);
		}
	}

}
