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

/// \note Terminates with a barrier.
void UpSweep512(local int* cache)
{
  /// \note for N = 512: k = log2(512) = 9
  const int k = 9;

  for(int d = 0; d < k; ++d)
  {
    // Calculate an index:
    int index = (LX + 1) * Pow2(d + 1) - 1;

    // Calculate the threshold that tells the current thread
    // whether it is allowed to run or not
    int threshold = 512;

    // If the current thread can run, we calculate the sum.
    if (index < threshold)
    {
      // The index of the other element to add.
      int other = index - Pow2(d);
      // Add the current value with the other value
      // And store it at the position of the current value.
      cache[index] += cache[other];
    }

    // Wait for all other threads to finish their iteration
    barrier(CLK_LOCAL_MEM_FENCE);
  }
}

/// \note Terminates with a barrier.
void DownSweep512(local int* cache)
{
  /// \note for N = 512: k = log2(512) = 9
  const int k = 9;

  for(int d = k; d >= 0; --d)
  {
    int index = (LX + 1) * Pow2(d + 1) - 1;

    // Calculate the threshold that tells the current thread
    // whether it is allowed to run or not
    int threshold = 512;

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

    // Wait for all other threads to finish their iteration
    barrier(CLK_LOCAL_MEM_FENCE);
  }
}

/// Calculates the prefix sum for the given \a data using the up-sweep and down-sweep techniques.
/// \note Size of \a cache must be 512 * sizeof(int).
/// \note Terminates with a barrier.
void PrefixSum512(global int* in, global int* out, local int* cache)
{
  // Preparation
  //////////////////////////////////////////////////////////////////////////
  // Copy data from global memory to local memory.
  cache[LX]       = in[LX];
  cache[LX + 256] = in[LX + 256];
  barrier(CLK_LOCAL_MEM_FENCE);

  // Up- and down sweep.
  //////////////////////////////////////////////////////////////////////////
  UpSweep512(cache);

  // Set the last value in the cache to 0 (required by the algorithm).
  if(LX == 0)
    cache[512 - 1] = 0;
  // Let all threads wait for local thread #0 before continuing.
  barrier(CLK_LOCAL_MEM_FENCE);

  DownSweep512(cache);

  // Finalization
  //////////////////////////////////////////////////////////////////////////
  // Copy local data back to global memory.
  out[LX]       = cache[LX];
  out[LX + 256] = cache[LX + 256];
  barrier(CLK_LOCAL_MEM_FENCE);
}

/// \brief Calculates prefix sums in a block-wise manner.
kernel void PrefixSum(global int* in_A, global int* out_B, local int* cache)
{
  int groupID = get_group_id(0);
  int offset = groupID * 512;
  // Calculate the prefix sum and store the result in the same array.
  PrefixSum512(in_A + offset, out_B + offset, cache + offset);
}
