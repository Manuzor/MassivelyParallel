
// More readable shortcuts for thread meta data on compute device
#define GX  get_global_id(0)   ///< Global x-coordinate
#define GY  get_global_id(1)   ///< Global y-coordinate
#define GNX get_global_size(0) ///< Global work size in x-direction
#define GNY get_global_size(1) ///< Global work size in y-direction
#define GN  (GNX * GNY)        ///< Total global work size

// More readable shortcuts for thread meta data on compute unit
#define LX  get_local_id(0)   ///< x-coordinate local to compute unit
#define LY  get_local_id(1)   ///< y-coordinate local to compute unit
#define LNX get_local_size(0) ///< Local work size in x-direction on compute unit
#define LNY get_local_size(1) ///< Local work size in y-direction on compute unit
#define LN  (LNX * LNY)       ///< Total local work size on compute unit

#define Pow2(x) (1 << (x)) ///< Calculates 2^x

int CalcIndexRowMajor(int width)   { return GY * width + GX; }
int CalcIndexColumnMajor(int width) { return GX * width + GY; }
int CalcIndex(int width) { return CalcIndexRowMajor(width); }

kernel void BlendX(int width, int height, global uchar4* pixels)
{
  pixels[CalcIndex(width)].x *= 0.9f;
  pixels[CalcIndex(width)].y *= 0.9f;
  pixels[CalcIndex(width)].z *= 0.9f;
  //pixels[CalcIndex(width)].w *= 0.9f;
}
