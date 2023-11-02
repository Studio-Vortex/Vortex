using Vortex;

namespace Sandbox {

	class RailedCamera: Entity
	{
		void OnCreate()
		{

		}

		void OnUpdate()
		{
			transform.Translate(transform.Forward * Time.DeltaTime);
		}
	}

}
