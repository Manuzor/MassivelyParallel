#include "PCH.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

#include <mpWrapper/Foundation/Startup.h>
#include <mpWrapper/Utilities/Console.h>

using std::swap;

// Calculates 2^x
#define Pow2(x) (1 << (x))

/// \brief Calculates the prefix sum iteratively on the CPU.
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

namespace RadixSort
{
  void CountValues(mpArrayPtr<cl_int> data,
                   cl_int byteNr,
                   cl_int* counts)
  {
    memset(counts, 0, 256 * sizeof(cl_int));

    auto shiftAmount = byteNr * 8; // 1 byte = 8 bits => multiply by 8.
    for (cl_int i = 0; i < data.m_uiCount; ++i)
    {
      auto val = (data[i] >> shiftAmount) & 255;
      ++counts[val];
    }
  }

  void InsertSorted(mpArrayPtr<cl_int> source, mpArrayPtr<cl_int> destination,
                    cl_int* prefix, cl_int byteNr)
  {
    auto shiftAmount = byteNr * 8; // 1 byte = 8 bits => multiply by 8.

    for(cl_int i = 0; i < source.m_uiCount; ++i)
    {
      auto val = (source[i] >> shiftAmount) & 255; // Only look at 1 byte.
      auto& index = prefix[val];
      destination[index] = source[i];
      ++index;
    }
  }

  /// \brief Radix sort implementation.
  ///        Sorting by the least significant digit (LSD) first.
  template<typename Type>
  void LSD(mpArrayPtr<Type> data)
  {
    MP_LogBlock("MSD");

    auto temp = new cl_int[data.m_uiCount];
    cl_int counts[256];
    cl_int prefix[256];
    for (cl_int byteNr = 0; byteNr < 4; ++byteNr)
    {
      CountValues(data, byteNr, counts);
      CalcPrefixSum(mpMakeArrayPtr(counts), mpMakeArrayPtr(prefix));
      if (byteNr % 2 == 0)
      {
        InsertSorted(data, mpMakeArrayPtr(temp, data.m_uiCount), prefix, byteNr);
      }
      else
      {
        InsertSorted(mpMakeArrayPtr(temp, data.m_uiCount), data, prefix, byteNr);
      }
    }

    //std::sort(data.m_Data, data.m_Data + data.m_uiCount);

    delete[] temp;
  }
}

/// \brief Prints the data in the given array with indices and column alignment.
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

  /// Number of elements to be processed.
  const cl_int N = 256;

  cl_int* inputData      = nullptr;
  cl_int* outputData_CPU = nullptr;
  cl_int* outputData_GPU = nullptr;

  mpPlatform Platform;
  mpDevice Device;

  mpContext Context;
  mpCommandQueue Queue;
  mpProgram Program_PrefixSum;
  mpKernel Kernel_PrefixSum;

  virtual void PostStartup() override
  {
    m_TimeRunning = mpTime::Now();

    inputData      = new cl_int[N];
    outputData_CPU = new cl_int[N];
    outputData_GPU = new cl_int[N];
  }

  virtual void PreShutdown() override
  {
    delete[] outputData_GPU;
    delete[] outputData_CPU;
    delete[] inputData;

    auto tElapsedTime = mpTime::Now() - m_TimeRunning;
    mpLog::Info("Finished execution in %f seconds\n", tElapsedTime.GetSeconds());
    printf("Press any key to quit . . .");
    mpUtilities::GetSingleCharacter();
  }

  virtual QuitOrContinue Run() override
  {
    mpLog::Info("N = %u", N);

    // Prepare input data.
    //////////////////////////////////////////////////////////////////////////
    for (size_t i = 0; i < N; ++i)
    {
      inputData[i] = (i + N/2) % N;
    }

    {
      MP_LogBlock("Input Data");
      //PrintData(mpMakeArrayPtr(inputData, N));
    }

    // Process data.
    //////////////////////////////////////////////////////////////////////////
    CPU(inputData);
    //GPU(inputData);

    return Quit;
  }

  void CPU(const cl_int* inputData)
  {
    MP_LogBlock("CPU");

    // Copy the input data and sort it.
    memcpy(outputData_CPU, inputData, N * sizeof(cl_int));
    {
      mpLog::Info("Running...");
      MP_Profile("CPU");

      RadixSort::LSD(mpMakeArrayPtr(outputData_CPU, N));
      //Hahn::radixsort((int*)outputData_CPU, (int)N);
    }
    PrintData(mpMakeArrayPtr(outputData_CPU, N));
  }

  void GPU(const cl_int* inputData)
  {
    MP_LogBlock("GPU");

    {
      MP_Profile("Initialization");
      Platform.Initialize();
      Device = mpDevice::GetGPU(Platform, 0);

      Context.Initialize(Device);
      Queue.Initialize(Context, Device);
      MP_Verify(Program_PrefixSum.LoadAndBuild(Context, Device, "Kernels/PrefixSum256.cl"));
      Kernel_PrefixSum.Initialize(Queue, Program_PrefixSum, "PrefixSum");
    }

    mpBuffer inputBuffer;
    inputBuffer.Initialize(Context,
                            mpBufferFlags::ReadOnly,
                            mpMakeArrayPtr(inputData, N));
    mpBuffer outputBuffer;
    outputBuffer.Initialize(Context,
                            mpBufferFlags::ReadWrite,
                            mpMakeArrayPtr(outputData_GPU, N));

    CalcPrefixSum256(inputBuffer, outputBuffer);

    outputBuffer.ReadInto(mpMakeArrayPtr(outputData_GPU, N), Queue);
    PrintData(mpMakeArrayPtr(outputData_GPU, N));
  }

  /// \brief Calculates the prefix sum for arrays of size 256 using 128 threads.
  void CalcPrefixSum256(mpBuffer& inputBuffer, mpBuffer& outputBuffer)
  {
    MP_LogBlock("Prefix Sum (GPU)");

    Kernel_PrefixSum.PushArg(inputBuffer);
    Kernel_PrefixSum.PushArg(outputBuffer);
    Kernel_PrefixSum.PushArg(mpLocalMemory<cl_int>(N));
    Kernel_PrefixSum.Execute(128);
  }
};

MP_EntryClass(Main);
