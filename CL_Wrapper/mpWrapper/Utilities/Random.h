#pragma once
#include <random>
#include "mpWrapper/Utilities/Range.h"

namespace mpInternal
{
  template<typename T>
  struct mpGetUniformDistributionTypeHelper
  {
    using Type = std::uniform_int_distribution<T>;
  };

  template<>
  struct mpGetUniformDistributionTypeHelper<cl_float>
  {
    using Type = std::uniform_real_distribution<cl_float>;
  };

  template<>
  struct mpGetUniformDistributionTypeHelper<double>
  {
    using Type = std::uniform_real_distribution<double>;
  };

  template<typename T>
  using mpUniformDistributionType = typename mpGetUniformDistributionTypeHelper<T>::Type;
}

namespace mpRandom
{
  using RandomEngineType = std::default_random_engine;
  using RandomDeviceType = std::random_device;

  MP_WrapperAPI RandomDeviceType& GetRandomDevice();

  class mpNumberGenerator
  {
  public:
    mpNumberGenerator() { RandomizeSeed(); }

    MP_ForceInline void RandomizeSeed() { SetSeed(GetRandomDevice()()); }
    MP_ForceInline void SetSeed(mpUInt32 uiNewSeed) { m_uiSeed = uiNewSeed; m_Engine.seed(uiNewSeed); }
    MP_ForceInline mpUInt32 GetSeed() const { return m_uiSeed; }
    MP_ForceInline void Reset() { SetSeed(GetSeed()); }

    template<typename NumberType>
    NumberType Generate(NumberType Min, NumberType Max)
    {
      return Generate(mpRange<NumberType>{ Min, Max });
    }

    template<typename NumberType>
    NumberType Generate(mpRange<NumberType> Range)
    {
      mpInternal::mpUniformDistributionType<NumberType> Generator(Range.m_Min, Range.m_Max);
      return Generator(m_Engine);
    }

  private:
    mpUInt32 m_uiSeed;
    RandomEngineType m_Engine;
  };
}
