using System;
using System.Collections.Generic;
using Sparky;

namespace Sandbox {

	public class RayTracingTest : Entity
	{
		private List<Vector3> t0Hits;
		private List<Vector3> t1Hits;

		public override void OnCreate()
		{
			t0Hits = new List<Vector3>();
			t1Hits = new List<Vector3>();
		}

		public override void OnUpdate(float delta)
		{
			int width = 160, height = 90;
			float aspectRatio = width / (float)height;
			float length = 2.0f;
			float radius = 0.5f;

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

					// Quadratic formula discriminant
					// b^2 - 4ac

					float discriminant = b * b - 4 * a * c;

					Vector4 rayColor = new Vector4(0, 0, 0, 0.2f);

					if (discriminant >= 0.0f)
						rayColor = new Vector4(1, 0, 1, 1);

					DebugRenderer.DrawLine(rayOrigin, rayOrigin + rayDirection * length, rayColor);
				}
			}

			DebugRenderer.Flush();
		}
	}

}
