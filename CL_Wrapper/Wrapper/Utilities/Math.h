#pragma once

template<typename Type>
struct mpMathType;

template<>
struct mpMathType<cl_float>
{
  MP_ForceInline static cl_float GetHugepsilon() { return 0.1f; }
  MP_ForceInline static cl_float GetLargeEpsilon() { return 0.01f; }
  MP_ForceInline static cl_float GetDefaultEpsilon() { return 0.001f; }
  MP_ForceInline static cl_float GetSmallEpsilon() { return 0.0001f; }
  MP_ForceInline static cl_float CalcAbs(cl_float f) { return (cl_float)fabs(f); }
};

namespace mpMath
{
  /// \brief Convenient function that delegates the actual work to the mpMathType implementation.
  template<typename Type>
  MP_ForceInline Type CalcAbs(Type n)
  {
    return mpMathType<Type>::CalcAbs(n);
  }

  template<typename Type>
  MP_ForceInline bool IsEqual(Type Lhs, Type Rhs, Type Epsilon = mpMathType<Type>::GetDefaultEpsilon())
  {
    auto Diff = mpMathType<Type>::CalcAbs(Lhs - Rhs);
    return Diff < Epsilon;
  }
}

