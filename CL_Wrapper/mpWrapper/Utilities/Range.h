#pragma once

template<typename Type>
struct mpRangeTemplate
{
  MP_ForceInline static mpRangeTemplate ZeroToMax() { return mpRangeTemplate{ 0, std::numeric_limits<Type>::max() }; }
  MP_ForceInline static mpRangeTemplate ZeroToOne() { return mpRangeTemplate{ 0, 1 }; }


  Type m_Min;
  Type m_Max;

  mpRangeTemplate() : m_Min(0), m_Max(0) {}

  mpRangeTemplate(Type Min, Type Max) : m_Min(Min), m_Max(Max) {}
};

template<typename Type>
using mpRange = mpRangeTemplate<Type>;
