using Sparky;

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
			float distanceToPlayer = Vector3.Distance(transform.Translation, player.transform.Translation);
			if (distanceToPlayer <= 3f)
			{
				if (Input.IsKeyDown(KeyCode.E))
				{
					Open();
					GetComponent<AudioSource>().Play();
					FindEntityByName("Flashbang Sound").GetComponent<AudioSource>().Play();
				}

				helpText.SetActive(true);
			}
			else
			{
				helpText.SetActive(false);
			}

			if (m_ShouldOpen)
				RotateDoor();
		}

		public void Open()
		{
			m_ShouldOpen = true;
		}

		void RotateDoor()
		{
			if (transform.Rotation.Y < stopAngle)
			{
				transform.Rotate(0, rotationSpeed * Time.DeltaTime, 0);
				rotationSpeed -= Time.DeltaTime * 3.0f;
			}
		}
	}

}
