using Vortex;

namespace Sandbox {

	public class RayTracingTest : Entity
	{
		protected override void OnCreate()
		{
		}

		protected override void OnUpdate(float delta)
		{
			int width = 160, height = 90;
			float aspectRatio = width / (float)height;
			float length = 3.0f;
			float radius = 0.5f;
			Vector3 sphereOrigin = Vector3.Zero;
			Vector2 quadSize = new Vector2(0.05f);
			Vector4 quadColor = Color.Green4;

			DebugRenderer.BeginScene();

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					Vector2 coord = new Vector2(x / (float)width, y / (float)height);
					coord = coord * 2.0f - 1.0f; // -1 -> 1

					coord.X *= aspectRatio;

					Vector3 rayOrigin = new Vector3(0.0f, 0.0f, 2.0f);
					Vector3 rayDirection = new Vector3(coord.X, coord.Y, -1.0f);

					float a = Vector3.Dot(rayDirection, rayDirection);
					float b = 2.0f * Vector3.Dot(rayOrigin, rayDirection);
					float c = Vector3.Dot(rayOrigin, rayOrigin) - radius * radius;

					float discriminant = b * b - 4 * a * c;

					Vector4 rayColor = new Vector4(0, 0, 0, 0.2f);

					if (discriminant >= 0.0f)
					{
						float[] t = new float[] {
							(-b - Mathf.Sqrt(discriminant)) / (2.0f * a),
							(-b + Mathf.Sqrt(discriminant)) / (2.0f * a)
						};

						for (uint i = 0; i < 2; i++)
						{
							Vector3 hitPosition = rayOrigin + rayDirection * t[i];
							Vector3 normal = hitPosition - sphereOrigin;
							normal.Normalize();
							Vector3 n = normal * 0.5f + 0.5f;
							DebugRenderer.DrawQuadBillboard(hitPosition, quadSize, new Vector4(n.X, n.Y, n.Z, 1.0f));
						}

						rayColor = new Vector4(1, 0, 1, 1);
					}

					//DebugRenderer.DrawLine(rayOrigin, rayOrigin + rayDirection * length, rayColor);
				}
			}

			DebugRenderer.Flush();
		}
	}

}
