#pragma once
#include <random>

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

    template<typename IntegerType = cl_int>
    IntegerType GenerateInteger(IntegerType iMin = 0, IntegerType iMax = std::numeric_limits<IntegerType>::max())
    {
        std::uniform_int_distribution<IntegerType> Generator(iMin, iMax);
        return Generator(m_Engine);
    }

    template<typename FloatType = cl_float>
    FloatType GenerateFloat(FloatType fMin = 0.0f, FloatType fMax = 1.0f)
    {
        std::uniform_real_distribution<FloatType> Generator(fMin, fMax);
        return Generator(m_Engine);
    }

  private:
    mpUInt32 m_uiSeed;
    RandomEngineType m_Engine;
  };
}
