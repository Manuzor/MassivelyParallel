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
  template<typename ResultType = cl_float>
  MP_ForceInline static ResultType CalcAbs(cl_float f) { return (ResultType)std::abs(f); }
  template<typename ResultType = cl_float>
  MP_ForceInline static ResultType CalcFloor(cl_float f) { return (ResultType)std::floor(f); }
  template<typename ResultType = cl_float>
  MP_ForceInline static ResultType CalcCeil(cl_float f) { return (ResultType)std::ceil(f); }
  template<typename ResultType = cl_float>
  MP_ForceInline static ResultType CalcRound(cl_float f) { return (ResultType)std::round(f); }
};

template<>
struct mpMathType<double>
{
  MP_ForceInline static double GetHugepsilon() { return 0.01; }
  MP_ForceInline static double GetLargeEpsilon() { return 0.001; }
  MP_ForceInline static double GetDefaultEpsilon() { return 0.0001; }
  MP_ForceInline static double GetSmallEpsilon() { return 0.00001; }
  template<typename ResultType = double>
  MP_ForceInline static ResultType CalcAbs(double f) { return (ResultType)std::abs(f); }
  template<typename ResultType = double>
  MP_ForceInline static ResultType CalcFloor(double f) { return (ResultType)std::floor(f); }
  template<typename ResultType = double>
  MP_ForceInline static ResultType CalcCeil(double f) { return (ResultType)std::ceil(f); }
  template<typename ResultType = double>
  MP_ForceInline static ResultType CalcRound(double f) { return (ResultType)std::round(f); }
};

namespace mpMath
{
  /// \brief Convenient function that delegates the actual work to the mpMathType implementation.
  template<typename Type>
  MP_ForceInline Type CalcAbs(Type n) { return mpMathType<Type>::CalcAbs(n); }

  template<typename Type>
  MP_ForceInline Type CalcRound(Type n) { return mpMathType<Type>::CalcRound(n); }

  template<typename Type>
  MP_ForceInline Type CalcFloor(Type n) { return mpMathType<Type>::CalcFloor(n); }

  template<typename Type>
  MP_ForceInline Type CalcCeil(Type n) { return mpMathType<Type>::CalcCeil(n); }

  template<typename Type>
  MP_ForceInline bool IsEqual(Type Lhs, Type Rhs, Type Epsilon = mpMathType<Type>::GetDefaultEpsilon())
  {
    auto Diff = mpMathType<Type>::CalcAbs(Lhs - Rhs);
    return Diff < Epsilon;
  }
}

