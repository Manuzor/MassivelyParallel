
kernel void helloworld(global char* in, global char* out)
{
  int num = get_global_id(0);
  out[num] = in[num] + 1;
}
