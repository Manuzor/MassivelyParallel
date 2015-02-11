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

#define Exp2(x) (1 << (x)) // Raises 2 to the power of `x`.
#define BlockSize (2 * LN) // The block size of the prefix sum algorithm.
#define Log2i(i) ((int)log2((float)(i))) // Base 2 logarithm for integers.

/// \note Terminates with a barrier.
inline void UpSweep(local int* cache)
{
  /// \note for BlockSize == 512: k = log2(512) = 9
  const int k = Log2i(BlockSize);

  for(int d = 0; d < k; ++d)
  {
    // Calculate an index:
    int index = (LX + 1) * Exp2(d + 1) - 1;

    // If the current thread can run, we calculate the sum.
    if (index < BlockSize)
    {
      // The index of the other element to add.
      int other = index - Exp2(d);
      // Add the current value with the other value
      // And store it at the position of the current value.
      cache[index] += cache[other];
    }

    // Wait for all other threads to finish their iteration
    barrier(CLK_LOCAL_MEM_FENCE);
  }
}

/// \note Terminates with a barrier.
inline void DownSweep(local int* cache)
{
  /// \note for BlockSize == 512: k = log2(512) = 9
  const int k = Log2i(BlockSize);

  for(int d = k; d >= 0; --d)
  {
    int index = (LX + 1) * Exp2(d + 1) - 1;

    // If the current thread can run, we calculate the sum.
    if(index < BlockSize)
    {
      // The index of the other element to add.
      int other = index - Exp2(d);

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

/// \brief Calculates the prefix sum for the given \a data using the up-sweep and down-sweep techniques.
///
/// \pre Size of \a in and \a out must be at least BlockSize.
/// \pre Size of \a cache must be BlockSize.
///
/// \note Terminates with a barrier.
void PrefixSumBlock(global int* in, global int* out, local int* cache)
{
  // Preparation
  // ===========
  // Copy data from global memory to local memory.
  cache[LX]      = in[LX];
  cache[LX + LN] = in[LX + LN];
  barrier(CLK_LOCAL_MEM_FENCE);

  // UpSweep
  // =======
  UpSweep(cache);

  // Set the last value in the cache to 0 (required by the algorithm).
  if(LX == 0)
    cache[BlockSize - 1] = 0;
  // Let all threads wait for local thread #0 before continuing.
  barrier(CLK_LOCAL_MEM_FENCE);

  // DownSweep
  // =========
  DownSweep(cache);

  // Finalization
  // ============
  // Copy local data back to global memory.
  out[LX]      = cache[LX];
  out[LX + LN] = cache[LX + LN];
  barrier(CLK_LOCAL_MEM_FENCE);
}

////////////////////////////////////////////////////////////////////////////////
/// Kernels ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// \brief Calculates prefix sums in a block-wise manner.
/// \pre Size of \a in_A must be a multiple of BlockSize.
kernel void PrefixSum(global int* in_A, global int* out_B,
                      local int* cache)
{
  int offset = get_group_id(0) * BlockSize;
  // Calculate the prefix sum and store the result in the same array.
  PrefixSumBlock(in_A  + offset,
                 out_B + offset,
                 cache);
}

kernel void PrefixSumHelper(global int* in_A, global int* in_B,
                            global int* out_C, int blockSize)
{
  int inIndex = ((GX + 1) * blockSize) - 1;
  out_C[GX] = in_A[inIndex] + in_B[inIndex];
}

kernel void PrefixSumReduce(global int* in_B, global int* in_D)
{
  in_B += get_group_id(0) * BlockSize; // += local offset
  in_B[LX]      += in_D[get_group_id(0)];
  in_B[LX + LN] += in_D[get_group_id(0)];
}
