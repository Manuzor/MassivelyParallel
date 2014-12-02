
// Calculates 2^x
#define Pow2(x) (1 << (x))
#define N 256

void SingleThreaded(local int* data, int k)
{
  if(get_global_id(0) == 0)
  {
    // Up-Sweep Phase
    //////////////////////////////////////////////////////////////////////////
    for (int d = 0; d < k; ++d)
    {
      for (int i = Pow2(d + 1) - 1; i < N; i += Pow2(d + 1))
      {
        data[i] += data[i - Pow2(d)];
      }
    }

    // Down-Sweep Phase
    //////////////////////////////////////////////////////////////////////////
    data[N - 1] = 0;
    for (int d = k; d > 0; --d)
    {
      for (int i = Pow2(d) - 1; i < N; i += Pow2(d))
      {
        int tmp = data[i];
        data[i] += data[i - Pow2(d - 1)];
        data[i - Pow2(d - 1)] = tmp;
      }
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
}

void MultiThreaded(local int* data, int k)
{
  // Up-Sweep Phase
  //////////////////////////////////////////////////////////////////////////
  for (int d = 0; d < k; ++d)
  {
    int index = Pow2(d + 1) * (get_global_id(0) + 1);
    if (index < N)
    {
      data[index] += data[index - Pow2(d)];
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  // Down-Sweep Phase
  //////////////////////////////////////////////////////////////////////////
  if (get_global_id(0) == 0)
    data[N - 1] = 0;

  barrier(CLK_LOCAL_MEM_FENCE);
  for (int d = k; d > 0; --d)
  {
    int index = Pow2(d) * (get_global_id(0) + 1);
    if (index < N)
    {
      int tmp = data[index];
      data[index] += data[index - Pow2(d - 1)];
      data[index - Pow2(d - 1)] = tmp;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
  }
}

kernel void CalculatePrefixSum(global int* in_data, int k)
{
  local int data[N];
  data[get_global_id(0)] = in_data[get_global_id(0)];
  barrier(CLK_LOCAL_MEM_FENCE);

  //SingleThreaded(data, k);
  MultiThreaded(data, k);

  in_data[get_global_id(0)] = data[get_global_id(0)];
}
