using System;
using System.Collections.Generic;
using Sparky;

namespace Sandbox {

	public class RayTracingTest : Entity
	{
		private List<Vector3> t0Hits;
		private List<Vector3> t1Hits;

		protected override void OnCreate()
		{
			t0Hits = new List<Vector3>();
			t1Hits = new List<Vector3>();
		}

		protected override void OnUpdate(float delta)
		{
			int width = 160, height = 90;
			float aspectRatio = width / (float)height;
			float length = 3.0f;
			float radius = 0.5f;
			Vector2 quadSize = new Vector2(.05f);
			Vector4 quadColor = new Vector4(0, 1, 0, 1);

			t0Hits.Clear();
			t1Hits.Clear();

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
						float t0 = (-b - (float)Math.Sqrt(discriminant)) / (2.0f * a);
						float t1 = (-b + (float)Math.Sqrt(discriminant)) / (2.0f * a);

						{
							Vector3 hitPosition = rayOrigin + rayDirection * length;
							DebugRenderer.DrawQuadBillboard(hitPosition, quadSize, quadColor);
						}
						{
							Vector3 hitPosition = rayOrigin + rayDirection * length;
							DebugRenderer.DrawQuadBillboard(hitPosition, quadSize, quadColor);
						}

						rayColor = new Vector4(1, 0, 1, 1);
					}

					DebugRenderer.DrawLine(rayOrigin, rayOrigin + rayDirection * length, rayColor);
				}
			}

			DebugRenderer.Flush();
		}
	}

}
