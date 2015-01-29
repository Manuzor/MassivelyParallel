#include "PCH.h"
#include <mpWrapper/Foundation/Startup.h>
#include <mpWrapper/Utilities/Console.h>

#include <sstream>
#include <iomanip>

using std::swap;

// Calculates 2^x
#define Pow2(x) (1 << (x))

namespace UpDown
{
  template<typename Type>
  void UpSweep(mpArrayPtr<Type> in_Data)
  {
    const auto N = (cl_int)in_Data.m_uiCount;
    const auto k = (cl_int)mpMath::Log2((double)N);

    // d means depth in the "binary tree"
    for(cl_int d = 0; d < k; ++d)
    {
      for(cl_int i = Pow2(d + 1) - 1; i < N; i += Pow2(d + 1))
      {
        auto other = i - Pow2(d);
        in_Data[i] += in_Data[other];
      }
    }
  }

  template<typename Type>
  void DownSweep(mpArrayPtr<Type> in_Data)
  {
    const auto N = (cl_int)in_Data.m_uiCount;
    const auto k = (cl_int)mpMath::Log2((double)N);

    for(cl_int d = k; d > 0; --d)
    {
      for(cl_int i = Pow2(d) - 1; i < N; i += Pow2(d))
      {
        auto other = i - Pow2(d - 1);
        auto tmp = in_Data[i];
        in_Data[i] += in_Data[other];
        in_Data[other] = tmp;
      }
    }
  }

  template<typename Type>
  void CalcPrefixSum(mpArrayPtr<Type> in_Data, mpArrayPtr<Type> out_Data)
  {
    MP_Assert(out_Data.m_uiCount >= in_Data.m_uiCount, "The out-buffer is not big enough.");

    const auto N = (cl_int)in_Data.m_uiCount;
    if (N <= 0)
      return;

    MP_Assert(mpMath::IsPowerOf2(N), "The data size must be a power of 2.");

    // Copy over the data so we can work on stuff in-place.
    memcpy(out_Data.m_Data, in_Data.m_Data, in_Data.m_uiCount * sizeof(Type));

    UpSweep(out_Data);
    out_Data[N - 1] = 0;
    DownSweep(out_Data);
  }
}

namespace Naive
{
  /// \brief Calculates the prefix sum in-place.
  template<typename Type>
  void CalcPrefixSum(mpArrayPtr<Type> in_Data)
  {
    MP_Assert(out_Data.m_uiCount >= in_Data.m_uiCount, "The out-buffer is not big enough.");

    const auto N = in_Data.m_uiCount;
    if (N <= 0)
      return;

    Type previous = 0;

    // Don't do this in the loop because we are doing an i-1 access within it.
    swap(in_Data[0], previous);
    for (size_t i = 1; i < N; ++i)
    {
      swap(in_Data[i], previous);
      in_Data[i] += in_Data[i - 1];
    }
  }

  /// \brief Calculated the prefix sum iteratively.
  template<typename Type>
  void CalcPrefixSum(mpArrayPtr<Type> in_Data, mpArrayPtr<Type> out_Data)
  {
    MP_Assert(out_Data.m_uiCount >= in_Data.m_uiCount, "The out-buffer is not big enough.");

    const auto N = in_Data.m_uiCount;
    if (N <= 0)
      return;

    out_Data[0] = 0;
    for (size_t i = 0; i < N - 1; ++i)
    {
      out_Data[i + 1] = out_Data[i] + in_Data[i];
    }
  }
}

template<typename Type>
void PrintData(mpArrayPtr<Type> Data, const size_t uiWidth = 10)
{
  //return;
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

      msg << ' ' << std::setw(4) << Data[index];
    }
    mpLog::Info("%4d - %4d:%s", i * uiWidth, i * uiWidth + j - 1, msg.str().c_str());
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
    mpKernel Kernel_CalculatePrefixSum;
    Kernel_CalculatePrefixSum.Initialize(Queue, Program, "PrefixSum");

    // Number of Integers
    const size_t N = 1024;
    mpLog::Info("N = %u", N);

    // Input
    //////////////////////////////////////////////////////////////////////////
    auto inputData = new cl_int[N];
    MP_OnScopeExit{ delete[] inputData; };

    for (size_t i = 0; i < N; ++i)
    {
      inputData[i] = 1;
    }
    mpBuffer inputBuffer;
    inputBuffer.Initialize(Context,
                           mpBufferFlags::ReadOnly,
                           mpMakeArrayPtr(inputData, N));

    {
      MP_LogBlock("Input Data");
      PrintData(mpMakeArrayPtr(inputData, N));
    }

    // Output Data
    //////////////////////////////////////////////////////////////////////////
    auto outputData_CPU_Naive = new cl_int[N];
    MP_OnScopeExit{ delete[] outputData_CPU_Naive; };

    auto outputData_CPU_UpDownSweep = new cl_int[N];
    MP_OnScopeExit{ delete[] outputData_CPU_UpDownSweep; };

    auto outputData_GPU = new cl_int[N];
    MP_OnScopeExit{ delete[] outputData_GPU; };

    //////////////////////////////////////////////////////////////////////////
    /// Calculations
    //////////////////////////////////////////////////////////////////////////
    const bool naiveEnabled       = true;  // Enable or disable single-threaded naive algorithm on CPU.
    const bool upDownSweepEnabled = false; // Enable or disable single-threaded up/down sweep on the CPU.
    const bool gpuEnabled         = true;  // Enable or disable multi-threaded up/down sweep on the GPU.

    const size_t numCycles = 1;

    // CPU Calculation
    //////////////////////////////////////////////////////////////////////////
    if (naiveEnabled)
    {
      MP_LogBlock("CPU: Naive (%u cycle/s)", numCycles);
      {
        mpLog::Info("Running...");
        MP_Profile("CPU: Naive");
        for (size_t i = 0; i < numCycles; ++i)
        {
          Naive::CalcPrefixSum(mpMakeArrayPtr(inputData, N),
                               mpMakeArrayPtr(outputData_CPU_Naive, N));
        }
      }
      PrintData(mpMakeArrayPtr(outputData_CPU_Naive, N));
    }

    if (upDownSweepEnabled)
    {
      MP_LogBlock("CPU: Up/Downsweep (%u cycle/s)", numCycles);
      {
        mpLog::Info("Running...");
        MP_Profile("CPU: Up/Downsweep");
        for (size_t i = 0; i < numCycles; ++i)
        {
          UpDown::CalcPrefixSum(mpMakeArrayPtr(inputData, N),
                                mpMakeArrayPtr(outputData_CPU_UpDownSweep, N));
        }
      }
      PrintData(mpMakeArrayPtr(outputData_CPU_UpDownSweep, N));
    }

    // GPU Calculation
    //////////////////////////////////////////////////////////////////////////
    if (gpuEnabled)
    {
      MP_LogBlock("GPU (%u cycle/s)", numCycles);
      mpBuffer inputBuffer;
      inputBuffer.Initialize(Context,
                             mpBufferFlags::ReadOnly,
                             mpMakeArrayPtr(inputData, N));
      mpBuffer outputBuffer;
      outputBuffer.Initialize(Context,
                              mpBufferFlags::ReadWrite,
                              mpMakeArrayPtr(outputData_GPU, N));

      size_t globalWorkSize[] = { N / 2 };
      size_t localWorkSize[]  = { 256 };

      {
        // In case the global work size is not a multiple of the local work size,
        // we adjust it to be so. => for LN = 512 and N = 123: GN = 512
        auto remainder = N % 256;
        if(remainder)
        {
          globalWorkSize[0] += localWorkSize[0] - remainder;
        }
      }

      mpLog::Info("Global work size: %u", globalWorkSize[0]);
      mpLog::Info("Local work size:  %u", localWorkSize[0]);

      {
        mpLog::Info("Running...");
        MP_Profile("GPU");
        for(size_t i = 0; i < numCycles; ++i)
        {
          Kernel_CalculatePrefixSum.PushArg(inputBuffer);
          Kernel_CalculatePrefixSum.PushArg(outputBuffer);
          Kernel_CalculatePrefixSum.PushArg(mpLocalMemory<cl_int>(N));

          Kernel_CalculatePrefixSum.Execute(mpMakeArrayPtr(globalWorkSize), mpMakeArrayPtr(localWorkSize));
        }
      }

      outputBuffer.ReadInto(mpMakeArrayPtr(outputData_GPU, N), Queue);
      PrintData(mpMakeArrayPtr(outputData_GPU, N));
    }

    return Quit;
  }
};

MP_EntryClass(Main);
