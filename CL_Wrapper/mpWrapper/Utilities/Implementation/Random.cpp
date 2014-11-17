#include "mpWrapper/PCH.h"
#include "mpWrapper/Utilities/Random.h"

static std::random_device g_RandomDevice;

mpRandom::RandomDeviceType& mpRandom::GetRandomDevice()
{
  return g_RandomDevice;
}
