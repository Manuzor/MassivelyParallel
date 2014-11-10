
kernel void add1(global int* in, global int* out)
{
  int num = get_global_id(0);
  out[num] = in[num] + 1;
}
