// These Kernels assume that matrix data (textures, etc) are given in row-major format.

// More readable shortcuts for thread meta data on compute device (not unit)
#define GX get_global_id(0)   ///< Global x-coordinate
#define GY get_global_id(1)   ///< Global y-coordinate
#define GW get_global_size(0) ///< Global work size in x-direction
#define GH get_global_size(1) ///< Global work size in y-direction
#define GN (GW * GH)          ///< Total global work size

#define ShiftFactorX 2
#define ShiftFactorY 2

/// Is either fma or mad
#define FMA_or_MAD fma

/// Calculates the index into the one-dimensional row-major data array using the given x and y coordinates
#define CalcIndex(x, y) ((y) * GW + (x))

//#define Index (GY * GW + GX)
#define Index CalcIndex(GX, GY)

#define HorizontallyShiftedIndex CalcIndex((GX + GW/2) % GW, GY)
#define VerticallyShiftedIndex   CalcIndex(GX, (GY + GH/2) % GH)
#define HVShiftedIndex           CalcIndex((GX + GW/2) % GW, (GY + GH/2) % GH)

#define AlphaX clamp(fabs(FMA_or_MAD(-2.0f * GX, 1.0f / (GW-1.0f), 1.0f)), 0.0f, 1.0f)
#define AlphaY clamp(fabs(FMA_or_MAD(-2.0f * GY, 1.0f / (GH-1.0f), 1.0f)), 0.0f, 1.0f)

// Helpers
//////////////////////////////////////////////////////////////////////////

float4 CalcPixelBlend(float4 originalPixel, float4 shiftedPixel, float alpha)
{
  // Apply 1-alpha to the original pixel
  // Apply alpha to the shifted pixel
  // return the sum of the results
  return originalPixel * (1.0f - alpha)
       + shiftedPixel  * alpha;
}

// Kernels
//////////////////////////////////////////////////////////////////////////

kernel void BlendX(global uchar4* in, global uchar4* out)
{
  const float4 originalPixel = convert_float4(in[Index]);
  const float4 shiftedPixel  = convert_float4(in[HorizontallyShiftedIndex]);
  const float4 resultPixel   = CalcPixelBlend(originalPixel, shiftedPixel, AlphaX);
  out[Index] = convert_uchar4(resultPixel);
}

kernel void BlendY(global uchar4* in, global uchar4* out)
{
  const float4 originalPixel = convert_float4(in[Index]);
  const float4 shiftedPixel  = convert_float4(in[VerticallyShiftedIndex]);
  const float4 resultPixel   = CalcPixelBlend(originalPixel, shiftedPixel, AlphaY);
  out[Index] = convert_uchar4(resultPixel);
}

/// \brief Blends the texture given in \a in in x- and y-directions and writes the result to \a out.
///
/// Nomenclature:
/// O - original pixel
/// H - horizontally shifted pixel
/// V - vertically shifted pixel
/// D - diagonally shited pixel, i.e. HV or VH
/// <-- - x-alpha mask
/// ^
/// | - y-alpha mask
/// ====================
/// Visualize the algorithm like this:
/// --- Step 1 and 2 ---
///   O <-- H   Blend the original pixel with the horizontally shifted pixel using the x-alpha mask
///   . . . .
///   . . . .
///   V <-- D   Blend the vertically shifted pixel with the diagonally shifted pixel using the x-alpha mask
/// --- Step 3 ---------
///   O'. . H
///   ^ . . .   Blend the blended original pixel with the blended vertically shifted pixel using the y-alpha mask
///   | . . .
///   V'. . D
/// --- Result ---------
///   O'' . H
///   . . . .
///   . . . .
///   V'. . D
/// O'' represents the desired result.
kernel void Blend(global uchar4* in, global uchar4* out)
{
  const float4 pixelO = convert_float4(in[Index]);
  const float4 pixelH = convert_float4(in[HorizontallyShiftedIndex]);
  const float4 pixelV = convert_float4(in[VerticallyShiftedIndex]);
  const float4 pixelD = convert_float4(in[HVShiftedIndex]);

  // Naming pattern:
  // pixel(source)(target)(blend mask)
  // e.g.: pixelOHX => blended original pixel with horizontally shifted pixel using the x-alpha mask

  const float4 pixelOHX = CalcPixelBlend(pixelO, pixelH, AlphaX);
  const float4 pixelVDX = CalcPixelBlend(pixelV, pixelD, AlphaX);
  const float4 resultPixel = CalcPixelBlend(pixelOHX, pixelVDX, AlphaY);

  out[Index] = convert_uchar4(resultPixel);
}
