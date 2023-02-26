namespace Vortex {

	public enum Space
	{
		Local, World,
	}

	public enum ProjectionType
	{
		Perspective, Orthographic,
	}

	public enum LightType
	{
		Directional, Point, Spot,
	}

	public enum MeshType
	{
		Cube, Sphere, Capsule, Cone, Cylinder, Plane, Torus,
	}

	public enum RigidBodyType
	{
		Static, Dynamic,
	}

	public enum RigidBody2DType
	{
		Static, Dynamic, Kinematic,
	}

	public enum ForceMode
	{
		Force,          // A standard force, using Force = mass * distance / time^2
		Impulse,        // An Impulse force, using Force = mass * distance / time
		VelocityChange, // An Impulse that ignores the objects mass, e.g Force = distance / time
		Acceleration,   // A constant force, not accounting for mass, e.g Force = distance / time^2
	}

	public enum ActorLockFlag : uint
	{
		TranslationX = 1 << 0,
		TranslationY = 1 << 1,
		TranslationZ = 1 << 2,
		Translation = TranslationX | TranslationY | TranslationZ,

		RotationX = 1 << 3,
		RotationY = 1 << 4,
		RotationZ = 1 << 5,
		Rotation = RotationX | RotationY | RotationZ,
	}

	public enum CollisionDetectionType
	{
		Discrete,
		Continuous,
		ContinuousSpeculative,
	}

	public enum NonWalkableMode
	{
		PreventClimbing,
		PreventClimbingAndForceSliding,
	}

	public enum CapsuleClimbMode
	{
		Easy,
		Constrained,
	}

	public enum CombineMode
	{
		Average,
		Max,
		Min,
		Multiply,
	}

}
