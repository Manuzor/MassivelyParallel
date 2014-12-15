// These Kernels assume that matrix data (textures, etc) are given in row-major format.

// More readable shortcuts for thread meta data on compute device (not unit)
#define GX get_global_id(0)   ///< Global x-coordinate
#define GY get_global_id(1)   ///< Global y-coordinate
#define GW get_global_size(0) ///< Global work size in x-direction
#define GH get_global_size(1) ///< Global work size in y-direction
#define GN (GW * GH)          ///< Total global work size

#define ShiftFactorX 2
#define ShiftFactorY 2

/// Calculates the index into the one-dimensional row-major data array using the given x and y coordinates
int CalcIndex(int x, int y) { return y * GW + x; }

//#define Index (GY * GW + GX)
#define Index CalcIndex(GX, GY)

#define HorizontallyShiftedIndex CalcIndex((GX + GW/2) % GW, GY)
#define VerticallyShiftedIndex   CalcIndex(GX, (GY + GH/2) % GH)

#define AlphaX clamp(fabs(fma(-2.0f * GX, 1.0f / (GW-1.0f), 1.0f)), 0.0f, 1.0f)
#define AlphaY clamp(fabs(fma(-2.0f * GY, 1.0f / (GH-1.0f), 1.0f)), 0.0f, 1.0f)

// Kernels
//////////////////////////////////////////////////////////////////////////

kernel void BlendX(global uchar4* in, global uchar4* out)
{
  const float4 pixelA = convert_float4(in[Index                   ]) * (1.0f - AlphaX);
  const float4 pixelB = convert_float4(in[HorizontallyShiftedIndex]) * AlphaX;

  out[Index] = convert_uchar4(pixelA + pixelB);
}

kernel void BlendY(global uchar4* in, global uchar4* out)
{
  const float4 pixelA = convert_float4(in[Index                 ]) * (1.0f - AlphaY);
  const float4 pixelB = convert_float4(in[VerticallyShiftedIndex]) * AlphaY;

  out[Index] = convert_uchar4(pixelA + pixelB);
}
