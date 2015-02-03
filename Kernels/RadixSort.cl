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

#define MIN(x, y) ((x) < (y) ? (x) : (y))

inline uchar GetSingleByte(int value, int byteNr)
{
  return (value >> (byteNr * 8)) & 0xFF;
}

/// \pre counts == int[32][256] == int[8192]
kernel void CalcStatistics(global int* data, int size, int byteNr, global int* out, local int* counts)
{
  // Compute the correct offset for the current thread group.
  //////////////////////////////////////////////////////////////////////////
  {
    // All work items will look at 256 items (or less, if their index is out of bounds).
    // For LN = 32 => offset = groupID * 32 * 256 = groupID * 8192
    // Also, if size <= 8192 => maxGroupID = 0 => offset = 0
    int offset = get_group_id(0) * LN * 256;
    data += offset;
    size += offset;
    out  += get_group_id(0) * 256;
  }

  // Initialize `counts` to 0.
  //////////////////////////////////////////////////////////////////////////
  for(int i = 0; i < 256; i++)
  {
    counts[LX * 256 + i] = 0;
  }

  // Initialize `out` to 0.
  //////////////////////////////////////////////////////////////////////////
  for(int i = 0; i < 256 / LN; i++)
  {
    const int index = LX + LN * i;
    out[index] = 0;
  }

  barrier(CLK_LOCAL_MEM_FENCE);

  // Count the values.
  //////////////////////////////////////////////////////////////////////////
  for(int i = 0; i < 256; i++)
  {
    // LX + 32 * i   = index
    // ---------------------
    //  0 + 32 * 0   = 0
    //  0 + 32 * 1   = 32
    //  0 + 32 * 2   = 64
    // ...
    // 31 + 32 * 0   = 31
    // 31 + 32 * 1   = 63
    // 31 + 32 * 2   = 95
    // ...
    // 31 + 32 * 255 = 8191
    const int index = LX + 32 * i;
    if(index < size)
    {
      uchar byte = GetSingleByte(data[index], byteNr);
      counts[LX * 256 + byte]++;
    }
  }

  // Accumulate results and write to `out`.
  //////////////////////////////////////////////////////////////////////////
  for(int i = 0; i < 256; i++)
  {
    const int localCount = counts[LX * 256 + i];
    global int* targetAddress = out + i;

    // Add the local count to the current count at target address.
    atomic_add(targetAddress, localCount);
  }
}

/// \brief Will reduce the given data, divided in sections of LN, to the first section of data.
///
/// \param sectionCount The number of sections to reduce.
///
/// \pre size of cache == LN
/// \pre sectionCount > 1
/// \remark should work for any work size but LN == 256 is expected.
kernel void ReduceStatistics(global int* data, int sectionCount)
{
  // Initialze the accumulation variable for the local thread.
  //////////////////////////////////////////////////////////////////////////
  int sum = 0;

  // Accumulate all results in `sum`.
  //////////////////////////////////////////////////////////////////////////
  for(int i = 0; i < sectionCount; ++i)
  {
    sum += data[LX + i * 256];
  }

  // Copy result `sum` back to global memory.
  //////////////////////////////////////////////////////////////////////////
  data[LX] = sum;
}

kernel void InsertSorted(global int* source, global int* destination, int size,
                         global int* prefixSums,
                         int byteNr,
                         local int* indices)
{
  // Copy from global to local memory.
  //////////////////////////////////////////////////////////////////////////
  indices[LX] = prefixSums[LX];


  // Wait for all threads to finish their copying.
  barrier(CLK_LOCAL_MEM_FENCE);

  // Do the actual insertion.
  //////////////////////////////////////////////////////////////////////////
  const int numSteps = (size + LN - 1) / LN;
  for(int i = 0; i < numSteps; i++)
  {
    // Index into `source` for the current thread.
    const int srcIndex = LX + i * LN;
    if(srcIndex < size)
    {
      // Pointer to the destination index. This one will be inceremented later within this loop.
      local int* pDstIndex = indices + GetSingleByte(source[srcIndex], byteNr);

      // Copy from `source` to `destination`.
      // Also increment the destination index atomically.
      destination[atomic_inc(pDstIndex)] = source[srcIndex];
    }
  }
}
