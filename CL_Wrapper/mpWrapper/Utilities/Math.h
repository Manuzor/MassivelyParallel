#pragma once

template<typename Type>
struct mpMathType;

template<>
struct mpMathType<cl_float>
{
  MP_ForceInline static cl_float GetHugeEpsilon()    { return 1e-1f; }
  MP_ForceInline static cl_float GetLargeEpsilon()   { return 1e-2f; }
  MP_ForceInline static cl_float GetDefaultEpsilon() { return 1e-3f; }
  MP_ForceInline static cl_float GetSmallEpsilon()   { return 1e-4f; }
  template<typename ResultType = cl_float>
  MP_ForceInline static ResultType CalcAbs(cl_float f) { return (ResultType)std::abs(f); }
  template<typename ResultType = cl_float>
  MP_ForceInline static ResultType CalcFloor(cl_float f) { return (ResultType)std::floor(f); }
  template<typename ResultType = cl_float>
  MP_ForceInline static ResultType CalcCeil(cl_float f) { return (ResultType)std::ceil(f); }
  template<typename ResultType = cl_float>
  MP_ForceInline static ResultType CalcRound(cl_float f) { return (ResultType)std::round(f); }
  MP_ForceInline static cl_float Max(cl_float fLhs, cl_float fRhs) { return fLhs > fRhs ? fLhs : fRhs; }
  MP_ForceInline static cl_float Min(cl_float fLhs, cl_float fRhs) { return fLhs < fRhs ? fLhs : fRhs; }
  MP_ForceInline static cl_float Sqrt(cl_float fValue) { return sqrtf(fValue); }
};

template<>
struct mpMathType<double>
{
  MP_ForceInline static double GetHugeEpsilon()    { return 1e-2; }
  MP_ForceInline static double GetLargeEpsilon()   { return 1e-3; }
  MP_ForceInline static double GetDefaultEpsilon() { return 1e-4; }
  MP_ForceInline static double GetSmallEpsilon()   { return 1e-5; }
  template<typename ResultType = double>
  MP_ForceInline static ResultType CalcAbs(double f) { return (ResultType)std::abs(f); }
  template<typename ResultType = double>
  MP_ForceInline static ResultType CalcFloor(double f) { return (ResultType)std::floor(f); }
  template<typename ResultType = double>
  MP_ForceInline static ResultType CalcCeil(double f) { return (ResultType)std::ceil(f); }
  template<typename ResultType = double>
  MP_ForceInline static ResultType CalcRound(double f) { return (ResultType)std::round(f); }
  MP_ForceInline static double Max(double fLhs, double fRhs) { return fLhs > fRhs ? fLhs : fRhs; }
  MP_ForceInline static double Min(double fLhs, double fRhs) { return fLhs < fRhs ? fLhs : fRhs; }
  MP_ForceInline static double Sqrt(double fValue) { return sqrt(fValue); }
};

namespace mpMath
{
  /// \brief Convenient function that delegates the actual work to the mpMathType implementation.
  template<typename Type> MP_ForceInline
  Type CalcAbs(Type n) { return mpMathType<Type>::CalcAbs(n); }

  template<typename Type> MP_ForceInline
  Type CalcRound(Type n) { return mpMathType<Type>::CalcRound(n); }

  template<typename Type> MP_ForceInline
  Type CalcFloor(Type n) { return mpMathType<Type>::CalcFloor(n); }

  template<typename Type> MP_ForceInline
  Type CalcCeil(Type n) { return mpMathType<Type>::CalcCeil(n); }

  template<typename Type> MP_ForceInline
  Type Max(Type lhs, Type rhs) { return mpMathType<Type>::Max(lhs, rhs); }

  template<typename Type> MP_ForceInline
  Type Min(Type lhs, Type rhs) { return mpMathType<Type>::Min(lhs, rhs); }

  MP_ForceInline
  cl_float Sqrt(cl_float value) { return mpMathType<cl_float>::Sqrt(value); }
  MP_ForceInline
  double Sqrt(double value) { return mpMathType<double>::Sqrt(value); }

  template<typename Type> MP_ForceInline
  Type Ln(Type value) { return std::log(value); }
  template<typename Type> MP_ForceInline
  Type Log10(Type value) { return std::log10(value); }
  template<typename Type> MP_ForceInline
  Type Log2(Type value) { return std::log2(value); }

  template<typename Type> MP_ForceInline
  Type Clamp(Type value, Type min, Type max)
  {
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  template<typename Type> MP_ForceInline
  bool IsEqual(Type Lhs, Type Rhs, Type Epsilon = mpMathType<Type>::GetSmallEpsilon())
  {
    auto Diff = CalcAbs(Lhs - Rhs);
    return Diff < Epsilon * Max(CalcAbs(Lhs), CalcAbs(Rhs));
  }

  /// Should only be used for signed integers.
  template<typename Type> MP_ForceInline
  bool IsPowerOf2(const Type x)
  {
    if (x < 1)
      return false;

    return ((x & (x - 1)) == 0);
  }
}
