using Vortex;

namespace Sandbox {

	class TikTok_00: Entity
	{
		Entity mCamera;
		Vector3 mOffset;

		void OnCreate()
		{
			mCamera = Scene.PrimaryCamera;
			mOffset = transform.Translation - mCamera.transform.Translation;
			mOffset.Y = 0f;
		}

		void OnUpdate()
		{
			transform.Translate(transform.Forward * 50f * Time.DeltaTime);
			mCamera.transform.Translation += (mOffset) * Time.DeltaTime;
		}
	}

}
