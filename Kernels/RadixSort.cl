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
  return (value << shiftAmount) & 255;
}

kernel void CalcStatistics(global int* in, int size, int byteNr, global int* out)
{
  local int counts[32][256]; // 8192
  const int shiftAmount = byteNr * 8;

  // Maximum index for local thread: LX + (32 * 256) - 1 == LX + 8191
  if(LX + 8191 < size)
  {
    // Initialize counts to 0.
    for(int i = 0; i < 256; i++)
    {
      counts[LX][i] = 0;
    }

    // Create counts for each thread.
    for(int i = 0; i < 256; ++i)
    {
      uchar byte = GetSingleByte(in[LX + i * 32], shiftAmount);
      counts[LX][byte]++;
    }

    // Combine thread results.
    for(int k = 0; k < 32; k++)
    {
      if(LX == 0)
        out[k] = 0;
      barrier(CLK_LOCAL_MEM_FENCE);
      for(int i = 0; i < 8; i++)
      {
        out[k] += counts[k][LX + i * 32];
      }
    }
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
