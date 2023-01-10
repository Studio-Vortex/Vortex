namespace Vortex {

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
		Acceleration    // A constant force, not accounting for mass, e.g Force = distance / time^2
	}

	public enum CollisionDetectionType
	{
		Discrete,
		Continuous,
		ContinuousSpeculative
	}

}
