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

    for (size_t i = 0; i < data.m_uiCount; ++i)
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
    for(size_t i = 0; i < source.m_uiCount; ++i)
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

    auto swapBuffer = new cl_int[data.m_uiCount];
    cl_int counts[256];
    cl_int prefix[256];
    for (cl_int byteNr = 0; byteNr < 4; ++byteNr)
    {
      CountValues(data, byteNr, counts);
      CalcPrefixSum(mpMakeArrayPtr(counts), mpMakeArrayPtr(prefix));
      if (mpMath::IsEven(byteNr))
      {
        InsertSorted(data, mpMakeArrayPtr(swapBuffer, data.m_uiCount), prefix, byteNr);
      }
      else
      {
        InsertSorted(mpMakeArrayPtr(swapBuffer, data.m_uiCount), data, prefix, byteNr);
      }
    }

    delete[] swapBuffer;
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
    {
      MP_ReportError("Unequal items detected.");
      return false;
    }
  }

  return true;
}

class Main : public mpApplication
{
  mpTime m_TimeRunning;

  mpRandom::mpNumberGenerator m_Rand;

  /// Number of elements to be processed.
  cl_int N = 1024; // 1 KiB

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

  void ReadUserInput()
  {
    std::string input;

    while(true)
    {
      std::cout << "...| Size of input data (Default = " << N << "): ";
      std::getline(std::cin, input);
      if(input.empty())
        break; // Empty line means: Use the default.
      try
      {
        N = std::stoi(input);
        // If input could be parsed successfully, break.
        break;
      }
      catch(...) { /* Invalid stoi call*/ }
    }
  }

  virtual void PostStartup() override
  {
    ReadUserInput();
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
    mpTime time;
    for (mpUInt32 i = 0; i < 1; ++i)
    {
      m_Rand.RandomizeSeed();
      MP_LogBlock("N = %u | RNG Seed = %u", N, m_Rand.GetSeed());
      {
        MP_Profile("Work Iteration");
        mpLog::Info("Running...");
        Work();
      }
    }

    return Quit;
  }

  void Work()
  {
    // Prepare input data.
    //////////////////////////////////////////////////////////////////////////
    mpLog::Info("Note: Input data is generated in the range of [0, 10000).");
    for (cl_int i = 0; i < N; ++i)
    {
      // Random value in range of [0, 10000).
      inputData[i] = m_Rand.Generate<cl_int>(0, 10000);
    }

    {
      MP_LogBlock("Input Data");
      PrintData(mpMakeArrayPtr(inputData, N));
    }

    memcpy(expectedResult, inputData, N * sizeof(cl_int));
    std::sort(expectedResult, expectedResult + N);

    {
      MP_LogBlock("Expected Result (std::sort)");
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

      MP_Verify(Program_PrefixSum.LoadAndBuild(Context, Device, "Kernels/PrefixSum.cl"));
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

    auto localSwapBuffer = new cl_int[N];

    // Used by InsertSorted to swap between source and target.
    mpBuffer swapBuffer;
    swapBuffer.Initialize(Context,
                          mpBufferFlags::ReadWrite,
                          N * sizeof(cl_int));

    mpBuffer outputBuffer;
    outputBuffer.Initialize(Context,
                            mpBufferFlags::ReadWrite,
                            numWorkGroups * 256 * sizeof(cl_int));

    mpBuffer prefixSumBuffer;
    prefixSumBuffer.Initialize(Context,
                               mpBufferFlags::ReadWrite,
                               256 * sizeof(cl_int));

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
      Kernel_PrefixSum.PushArg(prefixSumBuffer); // Output.
      Kernel_PrefixSum.PushArg(mpLocalMemory<cl_int>(256));
      Kernel_PrefixSum.Execute(128);

      // Insert sorted.
      //////////////////////////////////////////////////////////////////////////
      if(true)
      {
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

        Kernel_InsertSorted.PushArg(N);                          // Number of elements in Source/Destination.
        Kernel_InsertSorted.PushArg(prefixSumBuffer);            // Prefix sums.
        Kernel_InsertSorted.PushArg(byteNr);                     // Byte Nr.
        Kernel_InsertSorted.PushArg(mpLocalMemory<cl_int>(256)); // Local indices.

        // Note: This should be called with 256 threads but the algorithm is not stable.
        //       Running it with the warp size of the GPU will yield proper results.
        Kernel_InsertSorted.Execute(32);

        Queue.EnqueueBarrier();
      }
      else // Note: This is used for debugging purposes only.
      {
        // Do the insertion on the CPU.
        //////////////////////////////////////////////////////////////////////////
        // Copy from GPU memory.
        // =====================
        cl_int prefix[256];
        prefixSumBuffer.ReadInto(mpMakeArrayPtr(prefix), Queue);
        {
          MP_LogBlock("Prefix sums (Byte #%d)", byteNr);
          PrintData(mpMakeArrayPtr(prefix));
        }

        inputBuffer.ReadInto(mpMakeArrayPtr(outputData_GPU, N), Queue);
        swapBuffer.ReadInto(mpMakeArrayPtr(localSwapBuffer, N), Queue);

        // Do the insertion.
        // =================
        if (mpMath::IsEven(byteNr))
        {
          RadixSort::InsertSorted(mpMakeArrayPtr(outputData_GPU, N), mpMakeArrayPtr(localSwapBuffer, N),
                                  prefix, byteNr);
        }
        else
        {
          RadixSort::InsertSorted(mpMakeArrayPtr(localSwapBuffer, N), mpMakeArrayPtr(outputData_GPU, N),
                                  prefix, byteNr);
        }

        // Re-create buffers and copy results back to GPU memory.
        // ======================================================
        inputBuffer.Release();
        swapBuffer.Release();

        inputBuffer.Initialize(Context, mpBufferFlags::ReadWrite, mpMakeArrayPtr(outputData_GPU, N));
        swapBuffer.Initialize(Context, mpBufferFlags::ReadWrite, mpMakeArrayPtr(localSwapBuffer, N));
      }
    }

    inputBuffer.ReadInto(mpMakeArrayPtr(outputData_GPU, N), Queue);
    PrintData(mpMakeArrayPtr(outputData_GPU, N));
  }
};

MP_EntryClass(Main);
