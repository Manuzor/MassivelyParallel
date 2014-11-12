#include "Wrapper/PCH.h"
#include "Wrapper/Utilities/Time.h"

static double g_fInvQpcFrequency;

namespace
{
  struct RunOnStartup
  {
    RunOnStartup()
    {
      LARGE_INTEGER frequency;
      QueryPerformanceFrequency(&frequency);

      g_fInvQpcFrequency = 1.0 / double(frequency.QuadPart);
    }
  };
}

static RunOnStartup g_TimeInitializer;

mpTime mpTime::Now()
{
  LARGE_INTEGER temp;
  QueryPerformanceCounter(&temp);

  return mpTime::Seconds(double(temp.QuadPart) * g_fInvQpcFrequency);
}
