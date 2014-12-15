
// More readable shortcuts for thread meta data on compute device
#define GX  get_global_id(0)   ///< Global x-coordinate
#define GY  get_global_id(1)   ///< Global y-coordinate
#define GW get_global_size(0)  ///< Global work size in x-direction
#define GH get_global_size(1)  ///< Global work size in y-direction
#define GN  (GW * GH)          ///< Total global work size

// More readable shortcuts for thread meta data on compute unit
#define LX  get_local_id(0)   ///< x-coordinate local to compute unit
#define LY  get_local_id(1)   ///< y-coordinate local to compute unit
#define LNX get_local_size(0) ///< Local work size in x-direction on compute unit
#define LNY get_local_size(1) ///< Local work size in y-direction on compute unit
#define LN  (LNX * LNY)       ///< Total local work size on compute unit

#define Pow2(x) (1 << (x)) ///< Calculates 2^x

#define IndexRM (GY * GW + GX) ///< Row Major
#define IndexCM (GY * GW + GX) ///< Column Major
#define Index IndexRM

/// Helper function that scales a \a pixel (uchar3) with a \a factor (float).
uchar3 scalePixel(uchar3 pixel, float factor)
{
  return convert_uchar3(convert_float3(pixel) * factor);
}

kernel void BlendX(global uchar4* in, global uchar4* out)
{
  out[Index].xyz = scalePixel(in[Index].xyz, 0.9f);
  out[Index].w = in[Index].w;
}
