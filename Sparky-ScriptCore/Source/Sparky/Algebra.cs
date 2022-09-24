namespace Sparky
{

	public static class Algebra
	{
		public static Vector3 CrossProduct(Vector3 left, Vector3 right)
		{
			return InternalCalls.Algebra_CrossProductVec3(ref left, ref right);
		}
	}

}
