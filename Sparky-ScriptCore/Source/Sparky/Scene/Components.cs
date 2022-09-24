namespace Sparky
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class Transform : Component
	{
		public Vector3 Translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
				return translation;
			}

			set
			{
				InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
			}
		}

		public Vector3 Scale
		{
			get
			{
				InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
				return scale;
			}

			set
			{
				InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
			}
		}

		public Vector3 Rotation
		{
			get
			{
				InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 rotation);
				return rotation;
			}

			set
			{
				InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
			}
		}
	}

	public class Sprite : Component
	{
		public Vector4 Color
		{
			get
			{
				InternalCalls.SpriteComponent_GetColor(Entity.ID, out Vector4 color);
				return color;
			}

			set
			{
				InternalCalls.SpriteComponent_SetColor(Entity.ID, ref value);
			}
		}

		public float Scale
		{
			get
			{
				InternalCalls.SpriteComponent_GetScale(Entity.ID, out float scale);
				return scale;
			}

			set
			{
				InternalCalls.SpriteComponent_SetScale(Entity.ID, value);
			}
		}
	}

	public class RigidBody2D : Component
	{
		public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake)
		{
			InternalCalls.RigidBody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
		}
	}

}
