
kernel void CalculatePrefixSum(global int* in, global int* out)
{
  out[get_global_id(0)] = 0;
}
