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
  template<typename Type>
  inline unsigned char GetSingleByte(Type value, Type byteNr)
  {
    return (unsigned char)((value >> (byteNr * 8)) & 0xFF);
  }

  void CountValues(mpArrayPtr<cl_int> data,
                   cl_int byteNr,
                   cl_int* counts)
  {
    memset(counts, 0, 256 * sizeof(cl_int));

    for (cl_int i = 0; i < data.m_uiCount; ++i)
    {
      auto byte = GetSingleByte(data[i], byteNr);
      ++counts[byte];
    }
  }

  void InsertSorted(mpArrayPtr<cl_int> source,
                    mpArrayPtr<cl_int> destination,
                    cl_int* prefix,
                    cl_int byteNr)
  {
    for(cl_int i = 0; i < source.m_uiCount; ++i)
    {
      auto byte = GetSingleByte(source[i], byteNr);
      auto& index = prefix[byte];
      destination[index] = source[i];
      ++index;
    }
  }

  /// \brief Radix sort implementation.
  ///        Sorting by the least significant digit (LSD) first.
  template<typename Type>
  void LSD(mpArrayPtr<Type> data)
  {
    MP_LogBlock("LSD");

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

template<typename Type>
static bool AreEqual(mpArrayPtr<Type> lhs, mpArrayPtr<Type> rhs)
{
  if (lhs.m_uiCount != rhs.m_uiCount)
    return false;

  for (size_t i = 0; i < lhs.m_uiCount; ++i)
  {
    // Note: we circumvent bounds checking here because it is not necessary.
    if (lhs.m_Data[i] != rhs.m_Data[i])
      return false;
  }

  return true;
}

class Main : public mpApplication
{
  mpTime m_TimeRunning;

  mpRandom::mpNumberGenerator m_Rand;

  /// Number of elements to be processed.
  const cl_int N = 256;

  cl_int* inputData      = nullptr;
  cl_int* expectedResult = nullptr;
  cl_int* outputData_CPU = nullptr;
  cl_int* outputData_GPU = nullptr;

  mpPlatform Platform;
  mpDevice Device;
  mpContext Context;
  mpCommandQueue Queue;

  mpProgram Program_PrefixSum;
  mpKernel Kernel_PrefixSum;

  mpProgram Program_RadixSort;
  mpKernel Kernel_CalcStatistics;
  mpKernel Kernel_ReduceStatistics;
  mpKernel Kernel_InsertSorted;

  virtual void PostStartup() override
  {
    m_TimeRunning = mpTime::Now();

    inputData      = new cl_int[N];
    expectedResult = new cl_int[N];
    outputData_CPU = new cl_int[N];
    outputData_GPU = new cl_int[N];
  }

  virtual void PreShutdown() override
  {
    delete[] outputData_GPU;
    delete[] outputData_CPU;
    delete[] expectedResult;
    delete[] inputData;

    auto tElapsedTime = mpTime::Now() - m_TimeRunning;
    mpLog::Info("Finished execution in %f seconds\n", tElapsedTime.GetSeconds());
    printf("Press any key to quit . . .");
    mpUtilities::GetSingleCharacter();
  }

  virtual QuitOrContinue Run() override
  {
    //MP_LogLevelForScope(mpLogLevel::Success);
    for (mpUInt32 i = 42; i < 43; ++i)
    {
      MP_LogBlock("Random seed: %u", i);
      m_Rand.SetSeed(i);
      Work();
    }

    return Quit;
  }

  void Work()
  {
    mpLog::Info("N = %u", N);

    // Prepare input data.
    //////////////////////////////////////////////////////////////////////////
    for (size_t i = 0; i < N; ++i)
    {
      //inputData[i] = ((i + N/2) % N);
      //inputData[i] = 1;

      // Random value in range of [0, N).
      inputData[i] = m_Rand.Generate<cl_int>(0, N);
    }

    {
      MP_LogBlock("Input Data");
      PrintData(mpMakeArrayPtr(inputData, N));
    }

    memcpy(expectedResult, inputData, N * sizeof(cl_int));
    std::sort(expectedResult, expectedResult + N);

    {
      MP_LogBlock("Expected Result");
      PrintData(mpMakeArrayPtr(expectedResult, N));
    }

    // Process data on CPU.
    //////////////////////////////////////////////////////////////////////////
    if(false)
    {
      CPU(inputData);
      if(AreEqual(mpMakeArrayPtr(outputData_CPU, N), mpMakeArrayPtr(expectedResult, N)))
        mpLog::Success("CPU result is correct!");
      else
        mpLog::Error("CPU result is incorrect!");
    }

    // Process data on GPU.
    //////////////////////////////////////////////////////////////////////////
    if(true)
    {
      GPU(inputData);
      if(AreEqual(mpMakeArrayPtr(outputData_GPU, N), mpMakeArrayPtr(expectedResult, N)))
        mpLog::Success("GPU result is correct!");
      else
        mpLog::Error("GPU result is incorrect!");
    }
  }

  void CPU(const cl_int* inputData)
  {
    MP_LogBlock("CPU");

    // Copy the input data to our own buffer and sort it there.
    memcpy(outputData_CPU, inputData, N * sizeof(cl_int));
    cl_int counts[256];
    memset(counts, 0, sizeof(counts));

    {
      mpLog::Info("Running...");
      MP_Profile("CPU");

      RadixSort::LSD(mpMakeArrayPtr(outputData_CPU, N));
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

      MP_Verify(Program_RadixSort.LoadAndBuild(Context, Device, "Kernels/RadixSort.cl"));
      Kernel_CalcStatistics.Initialize(Queue, Program_RadixSort, "CalcStatistics");
      Kernel_ReduceStatistics.Initialize(Queue, Program_RadixSort, "ReduceStatistics");
      Kernel_InsertSorted.Initialize(Queue, Program_RadixSort, "InsertSorted");
    }

    const cl_int numWorkItems  = 32;
    const cl_int perWorkItem   = 256;
    const cl_int perWorkGroup  = perWorkItem * numWorkItems;
    const cl_int numWorkGroups = ((N + perWorkGroup - 1) / perWorkGroup);

    mpLog::Info("Number of Work Items:  %d", numWorkItems);
    mpLog::Info("Per Work Item:         %d", perWorkItem);
    mpLog::Info("Number of Work Groups: %d", numWorkGroups);
    mpLog::Info("Per Work Group:        %d", perWorkGroup);

    mpBuffer inputBuffer;
    inputBuffer.Initialize(Context,
                           mpBufferFlags::ReadWrite,
                           mpMakeArrayPtr(inputData, N));

    // Used by InsertSorted to swap between source and target.
    mpBuffer swapBuffer;
    swapBuffer.Initialize(Context,
                          mpBufferFlags::ReadWrite,
                          N * sizeof(cl_int));

    mpBuffer outputBuffer;
    outputBuffer.Initialize(Context,
                            mpBufferFlags::ReadWrite,
                            numWorkGroups * 256 * sizeof(cl_int));

    for(cl_int byteNr = 0; byteNr < 4; ++byteNr)
    {
      // Calculate statistics.
      //////////////////////////////////////////////////////////////////////////
      Kernel_CalcStatistics.PushArg(inputBuffer);
      Kernel_CalcStatistics.PushArg(cl_int(N));
      Kernel_CalcStatistics.PushArg(byteNr);
      Kernel_CalcStatistics.PushArg(outputBuffer);
      Kernel_CalcStatistics.PushArg(mpLocalMemory<cl_int>(perWorkGroup));
      Kernel_CalcStatistics.Execute(numWorkGroups * numWorkItems, numWorkItems);

      // Reduce statistics. This is only needed if numWorkGroups > 1.
      //////////////////////////////////////////////////////////////////////////
      if (numWorkGroups > 1)
      {
        Kernel_ReduceStatistics.PushArg(outputBuffer);
        Kernel_ReduceStatistics.PushArg(numWorkGroups);
        Kernel_ReduceStatistics.Execute(256);
      }

      // Create the prefix sum of the statistics.
      //////////////////////////////////////////////////////////////////////////
      Kernel_PrefixSum.PushArg(outputBuffer); // Input.
      Kernel_PrefixSum.PushArg(outputBuffer); // Output.
      Kernel_PrefixSum.Execute(128);

      // Insert sorted.
      //////////////////////////////////////////////////////////////////////////
      if(mpMath::IsEven(byteNr))
      {
        Kernel_InsertSorted.PushArg(inputBuffer); // Source.
        Kernel_InsertSorted.PushArg(swapBuffer);  // Destination.
      }
      else
      {
        Kernel_InsertSorted.PushArg(swapBuffer);  // Source.
        Kernel_InsertSorted.PushArg(inputBuffer); // Destination.
      }

      Kernel_InsertSorted.PushArg(N);            // Number of elements in Source/Destination.
      Kernel_InsertSorted.PushArg(outputBuffer); // Prefix sums.
      Kernel_InsertSorted.PushArg(byteNr);       // Byte Nr.
      Kernel_InsertSorted.Execute(256);
    }

    cl_int result[256];
    outputBuffer.ReadInto(mpMakeArrayPtr(result), Queue);
    PrintData(mpMakeArrayPtr(result));
  }
};

MP_EntryClass(Main);
