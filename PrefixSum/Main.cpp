#include "PCH.h"
#include <mpWrapper/Foundation/Startup.h>
#include <mpWrapper/Utilities/Console.h>

#include <sstream>
#include <iomanip>

using std::swap;

// Calculates 2^x
#define Pow2(x) (1 << (x))

template<typename Type>
void CalcPrefixSumParallelizable(mpArrayPtr<Type> in_Data)
{
  const auto N = in_Data.m_uiCount;
  MP_Assert(mpMath::IsPowerOf2(N), "The data size must be a power of 2.");
  const auto k = (size_t)mpMath::Sqrt((cl_float)N);

  for (size_t d = 0 ; d < k ; ++d)
  {
    // This loop can be parallelized
    for(size_t i = Pow2(d + 1) - 1; i < N; i += Pow2(d + 1))
    {
      in_Data[i] = in_Data[i] + in_Data[i - Pow2(d)];
    }
  }

  // Down-Sweep
  in_Data[N - 1] = 0;
  for (mpInt64 d = k - 1; d >= 0; --d)
  {
    // This loop can be parallelized
    for (size_t i = Pow2(d + 1) - 1; i < N ; i += Pow2(d + 1))
    {
      Type tmp = in_Data[i];
      in_Data[i] += in_Data[i - Pow2(d)];
      in_Data[i - Pow2(d)] = tmp;
    }
  }
}

template<typename Type>
void CalcPrefixSum(mpArrayPtr<Type> in_Data)
{
  const auto N = in_Data.m_uiCount;

  Type previous = 0;

  // Don't do this in the loop because we are doing i-1 within it.
  swap(in_Data[0], previous);
  for (size_t i = 1; i < N; ++i)
  {
    swap(in_Data[i], previous);
    in_Data[i] += in_Data[i - 1];
  }
}

template<typename Type>
void PrintData(mpArrayPtr<Type> Data, const size_t uiWidth = 10)
{
  const size_t uiHeight = Data.m_uiCount / uiWidth;
  for (size_t i = 0; i < uiHeight + 1; ++i)
  {
    std::stringstream msg;
    size_t j;
    for (j = 0; j < uiWidth; ++j)
    {
      auto index = i * uiWidth + j;
      if(index >= Data.m_uiCount)
        break;

      msg << ' ' << std::setw(3) << Data[index];
    }
    mpLog::Info("%3d - %3d:%s", i * uiWidth, i * uiWidth + j - 1, msg.str().c_str());
  }
}

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

    const size_t E = 8;
    const size_t N = Pow2(E); // 2 ^ E

    // Input
    //////////////////////////////////////////////////////////////////////////
    cl_int inputData[N];
    for (size_t i = 0; i < N; ++i)
    {
      inputData[i] = 2;
    }
    mpBuffer inputBuffer;
    inputBuffer.Initialize(Context,
                           mpBufferFlags::ReadOnly,
                           mpMakeArrayPtr(inputData));

    {
      MP_LogBlock("Input Data");
      PrintData(mpMakeArrayPtr(inputData));
    }

    // Output Data
    //////////////////////////////////////////////////////////////////////////
    cl_int outputData_CPU_Naive[N];
    cl_int outputData_CPU_UpDownSweep[N];
    cl_int outputData_GPU[N];
    for (size_t i = 0; i < N; ++i)
    {
      outputData_CPU_Naive[i] = inputData[i];
      outputData_CPU_UpDownSweep[i] = inputData[i];
      outputData_GPU[i] = inputData[i];
    }


    {
      MP_LogBlock("Output Data (CPU, Naive)");
      CalcPrefixSum(mpMakeArrayPtr(outputData_CPU_Naive));
      PrintData(mpMakeArrayPtr(outputData_CPU_Naive));
    }


    {
      MP_LogBlock("Output Data (CPU, Up/Downsweep)");
      CalcPrefixSumParallelizable(mpMakeArrayPtr(outputData_CPU_UpDownSweep));
      PrintData(mpMakeArrayPtr(outputData_CPU_UpDownSweep));
    }

    // GPU Calculation
    //////////////////////////////////////////////////////////////////////////
    mpBuffer outputBuffer;
    outputBuffer.Initialize(Context,
                           mpBufferFlags::ReadOnly,
                           mpMakeArrayPtr(outputData_GPU));

    Kernel.PushArg(inputBuffer);
    Kernel.PushArg(outputBuffer);
    Kernel.Execute(N);

    outputBuffer.ReadInto(mpMakeArrayPtr(outputData_GPU), Queue);

    {
      MP_LogBlock("Output Data (GPU)");
      PrintData(mpMakeArrayPtr(outputData_GPU));
    }

    return Quit;
  }
};

MP_EntryClass(Main);
