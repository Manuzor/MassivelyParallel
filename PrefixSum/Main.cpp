#include "PCH.h"
#include <mpWrapper/Foundation/Startup.h>
#include <mpWrapper/Utilities/Console.h>

#include <sstream>

class Main : public mpApplication
{
  mpTime m_TimeRunning;

  virtual void PostStartup() override
  {
    m_TimeRunning = mpTime::Now();
  }

  virtual void PreShutdown() override
  {
    auto tElapsedTime = mpTime::Now() - m_TimeRunning;
    mpLog::Info("Finished execution in %f seconds\n", tElapsedTime.GetSeconds());
    printf("Press any key to quit . . .");
    mpUtilities::GetSingleCharacter();
  }

  virtual QuitOrContinue Run() override
  {
    auto Platform = mpPlatform::Get();
    auto Device = mpDevice::GetGPU(Platform, 0);
    mpContext Context;
    Context.Initialize(Device);
    mpCommandQueue Queue;
    Queue.Initialize(Context, Device);
    mpProgram Program;
    MP_Verify(Program.LoadAndBuild(Context, Device, "Kernels/PrefixSum.cl"));
    mpKernel Kernel;
    Kernel.Initialize(Queue, Program, "CalculatePrefixSum");

    const size_t N = 256;

    // Input
    //////////////////////////////////////////////////////////////////////////
    cl_int inputData[N];
    for (size_t i = 0; i < N; ++i)
    {
      inputData[i] = i;
    }
    mpBuffer inputBuffer;
    inputBuffer.Initialize(Context,
                           mpBufferFlags::ReadOnly,
                           mpMakeArrayPtr(inputData));

    // Output
    //////////////////////////////////////////////////////////////////////////
    cl_int outputData[N];
    memset(outputData, 0, sizeof(outputData));
    mpBuffer outputBuffer;
    outputBuffer.Initialize(Context,
                           mpBufferFlags::ReadOnly,
                           mpMakeArrayPtr(outputData));

    Kernel.PushArg(inputBuffer);
    Kernel.PushArg(outputBuffer);
    Kernel.Execute(256);

    outputBuffer.ReadInto(mpMakeArrayPtr(outputData), Queue);

    const size_t uiWidth = 20;
    const size_t uiHeight = N / uiWidth;
    for (size_t i = 0; i < uiHeight + 1; ++i)
    {
      std::stringstream msg;
      size_t j;
      for (j = 0; j < uiWidth; ++j)
      {
        auto index = i * uiWidth + j;
        if(index >= N)
          break;

        msg << ' ' << outputData[index];
      }
      mpLog::Info("%3d - %3d:%s", i * uiWidth, i * uiWidth + j - 1, msg.str().c_str());
    }

    return Quit;
  }
};

MP_EntryClass(Main);
