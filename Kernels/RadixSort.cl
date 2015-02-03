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

inline uchar GetSingleByte(int value, int shiftAmount)
{
  return (value >> shiftAmount) & 0xFF;
}

// Assuming counts = int[32][256] = int[8192]
kernel void CalcStatistics(global int* in, int size, int byteNr, global int* out, local int* counts)
{
  // Initialize `counts` to 0.
  //////////////////////////////////////////////////////////////////////////
  for(int i = 0; i < 256; i++)
  {
    counts[LX * 256 + i] = 0;
  }

  // Initialize `out` to 0.
  //////////////////////////////////////////////////////////////////////////
  for(int i = 0; i < 8; i++)
  {
    int index = LX + 32 * i;
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
    int index = LX + 32 * i;
    if(index < size)
    {
      uchar byte = GetSingleByte(in[index], byteNr * 8);
      counts[LX * 256 + byte]++;
    }
  }

  // Accumulate results and write to `out`.
  //////////////////////////////////////////////////////////////////////////
  for(int i = 0; i < 256; i++)
  {
    int myLocalCount = counts[LX * 256 + i];
    global int* targetAddress = out + i; // == &out[i]

    // Add my local count to the current count at target address.
    atomic_add(targetAddress, myLocalCount);
  }
}

kernel void ReduceStatistics(global int* pBegin, int size)
{
  // Expecting LN = 256 but should work for any work size.

  global int* pEnd  = MIN(pBegin + LN, pBegin + size);
  global int* pData = pBegin + get_group_id(0) * LN;
  int sum = 0;

  while(pData < pEnd)
  {
    sum += *pData;
    pData += LN;
  }

  pData[LX] = sum;
}
