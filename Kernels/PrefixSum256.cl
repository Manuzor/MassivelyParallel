// More readable shortcuts forthread meta data on compute device (not unit)
#define GX get_global_id(0)   ///< Global x-coordinate
#define GY get_global_id(1)   ///< Global y-coordinate
#define GW get_global_size(0) ///< Global work size in x-direction
#define GH get_global_size(1) ///< Global work size in y-direction
#define GN (GW * GH)          ///< Total global work size

#define LX get_local_id(0)   ///< Local x-coordinate
#define LY get_local_id(1)   ///< Local y-coordinate
#define LW get_local_size(0) ///< Local work size in x-direction
#define LH get_local_size(1) ///< Local work size in y-direction
#define LN (LW * LH)         ///< Total local work size

#define Pow2(x) (1 << (x))

/// \note for N = 256: k = log2(256) = 8
constant const int k = 8;

/// \note Terminates with a barrier.
void UpSweep256(local int* cache)
{
  for(int d = 0; d < k; ++d)
  {
    // Calculate an index:
    const int index = (LX + 1) * Pow2(d + 1) - 1;

    // Calculate the threshold that tells the current thread
    // whether it is allowed to run or not
    const int threshold = 256;

    // If the current thread can run, we calculate the sum.
    if (index < threshold)
    {
      // The index of the other element to add.
      int other = index - Pow2(d);
      // Add the current value with the other value
      // And store it at the position of the current value.
      cache[index] += cache[other];
    }

    // Wait forall other threads to finish their iteration
    barrier(CLK_LOCAL_MEM_FENCE);
  }
}

/// \note Terminates with a barrier.
void DownSweep256(local int* cache)
{
  for(int d = k; d >= 0; --d)
  {
    const int index = (LX + 1) * Pow2(d + 1) - 1;

    // Calculate the threshold that tells the current thread
    // whether it is allowed to run or not
    const int threshold = 256;

    // If the current thread can run, we calculate the sum.
    if(index < threshold)
    {
      // The index of the other element to add.
      int other = index - Pow2(d);

      // Cache the current value...
      int previous = cache[index];

      // ... add the value at the other index to the current value...
      cache[index] += cache[other];

      // ... and overwrite the value at the other index with the cached value.
      cache[other] = previous;
    }

    // Wait forall other threads to finish their iteration
    barrier(CLK_LOCAL_MEM_FENCE);
  }
}

/// Calculates the prefix sum for the given \a data using the up-sweep and down-sweep techniques.
/// \note Size of \a cache must be 256 * sizeof(int).
/// \note Terminates with a barrier.
kernel void PrefixSum(global int* in, global int* out, local int* cache)
{
  // Preparation
  //////////////////////////////////////////////////////////////////////////
  // Copy data from global memory to local memory.
  cache[LX]       = in[LX];
  cache[LX + 128] = in[LX + 128];
  barrier(CLK_LOCAL_MEM_FENCE);
  return;

  // Up- and down sweep.
  //////////////////////////////////////////////////////////////////////////
  //UpSweep256(cache);

  // Set the last value in the cache to 0 (required by the algorithm).
  if(LX == 0)
    cache[256 - 1] = 0;
  // Let all threads wait for local thread #0 before continuing.
  barrier(CLK_LOCAL_MEM_FENCE);

  //DownSweep256(cache);

  // Finalization
  //////////////////////////////////////////////////////////////////////////
  // Copy local data back to global memory.
  out[LX]       = cache[LX];
  out[LX + 128] = cache[LX + 128];
  barrier(CLK_LOCAL_MEM_FENCE);
}
