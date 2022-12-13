using Vortex;

namespace Sandbox.DarkRoom {

	public class OpenDoor : Entity
	{
		public float rotationSpeed;
		public float stopAngle;

		Entity player;
		Entity helpText;
		bool m_ShouldOpen = false;

		protected override void OnCreate()
		{
			player = FindEntityByName("Player");
			helpText = FindEntityByName("Help Text");
		}

		protected override void OnUpdate(float delta)
		{
			helpText.SetActive(false);

			float distanceToPlayer = Vector3.Distance(transform.Translation, player.transform.Translation);
			if (distanceToPlayer <= 3f)
			{
				if (Input.IsKeyDown(KeyCode.E))
				{
					m_ShouldOpen = true;
					GetComponent<AudioSource>().Play();
					FindEntityByName("Flashbang Sound").GetComponent<AudioSource>().Play();
				}

				helpText.SetActive(true);
			}

			if (m_ShouldOpen)
			{
				RotateOpen();
				return;
			}
		}

		void RotateOpen()
		{
			if (transform.Rotation.Y < stopAngle)
			{
				transform.Rotate(0, rotationSpeed * Time.DeltaTime, 0);
				rotationSpeed -= Time.DeltaTime * 3.0f;
			}
		}
	}

}
