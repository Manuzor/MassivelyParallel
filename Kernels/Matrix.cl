
kernel void multiply(global float* lhs, global float* rhs, global float* result)
{
  int i = get_global_id(0);
  result[i] = lhs[i] + rhs[i];
}
