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
    mpLog::Debug("%4d - %4d:%s", i * uiWidth, i * uiWidth + j - 1, msg.str().c_str());
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

static mpRandom::mpNumberGenerator g_Rand;

class Main : public mpApplication
{
  mpTime m_TimeRunning;

  /// Number of elements to be processed.
  cl_int N = 8192;

  /// The number of elements that can be processed by a single work group.
  cl_int blockSize = 512;

  /// Enable or disable single-threaded naive algorithm on CPU.
  const bool naiveEnabled       = true;

  /// Enable or disable single-threaded up/down sweep on the CPU.
  const bool upDownSweepEnabled = false;

  /// Enable or disable multi-threaded up/down sweep on the GPU.
  const bool gpuEnabled         = true;

  cl_int* inputData                  = nullptr;
  cl_int* outputData_CPU_Naive       = nullptr;
  cl_int* outputData_CPU_UpDownSweep = nullptr;
  cl_int* outputData_GPU             = nullptr;

  mpPlatform Platform;
  mpDevice Device;
  mpContext Context;
  mpCommandQueue Queue;
  mpProgram Program;
  mpKernel Kernel_PrefixSum;
  mpKernel Kernel_PrefixSumHelper;
  mpKernel Kernel_PrefixSumReduce;

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

    while(true)
    {
      std::cout << "...| Block size (Default = " << blockSize << "): ";
      std::getline(std::cin, input);
      if(input.empty())
        break; // Empty line means: Use the default.
      try
      {
        blockSize = std::stoi(input);
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

    inputData                  = new cl_int[N];
    outputData_CPU_Naive       = new cl_int[N];
    outputData_CPU_UpDownSweep = new cl_int[N];
    outputData_GPU             = new cl_int[N + blockSize]; /// Allocate enough for the worst case.
  }

  virtual void PreShutdown() override
  {
    delete[] outputData_GPU;
    delete[] outputData_CPU_UpDownSweep;
    delete[] outputData_CPU_Naive;
    delete[] inputData;

    auto tElapsedTime = mpTime::Now() - m_TimeRunning;
    mpLog::Info("Finished execution in %f seconds\n", tElapsedTime.GetSeconds());
    printf("Press any key to quit . . .");
    mpUtilities::GetSingleCharacter();
  }

  virtual QuitOrContinue Run() override
  {
    mpLog::Info("N =          %u", N);
    mpLog::Info("Block Size = %u", blockSize);
    mpLog::Info("Note: Input data will be randomly generated in the range of [0, 20).");

    // Input
    //////////////////////////////////////////////////////////////////////////
    for (cl_int i = 0; i < N; ++i)
    {
      inputData[i] = g_Rand.Generate(0, 20);
    }

    {
      MP_LogBlock("Input Data");
      PrintData(mpMakeArrayPtr(inputData, N));
    }

    // CPU Calculation
    //////////////////////////////////////////////////////////////////////////
    if (naiveEnabled)
    {
      MP_LogBlock("CPU: Naive");
      {
        mpLog::Info("Running...");
        MP_Profile("CPU: Naive");
        Naive::CalcPrefixSum(mpMakeArrayPtr(inputData, N),
                              mpMakeArrayPtr(outputData_CPU_Naive, N));
      }
      PrintData(mpMakeArrayPtr(outputData_CPU_Naive, N));
    }

    if (upDownSweepEnabled)
    {
      MP_LogBlock("CPU: Up/Downsweep");
      {
        mpLog::Info("Running...");
        MP_Profile("CPU: Up/Downsweep");
        UpDown::CalcPrefixSum(mpMakeArrayPtr(inputData, N),
                              mpMakeArrayPtr(outputData_CPU_UpDownSweep, N));
      }
      PrintData(mpMakeArrayPtr(outputData_CPU_UpDownSweep, N));
    }

    // GPU Calculation
    //////////////////////////////////////////////////////////////////////////
    if (gpuEnabled)
      GPU(inputData);

    if (naiveEnabled && gpuEnabled &&
        AreEqual(mpMakeArrayPtr(outputData_CPU_Naive, N), mpMakeArrayPtr(outputData_GPU, N)))
    {
      mpLog::Success("Results for CPU (Naive) and GPU are equal!");
    }
    else if (upDownSweepEnabled && gpuEnabled &&
             AreEqual(mpMakeArrayPtr(outputData_CPU_UpDownSweep, N), mpMakeArrayPtr(outputData_GPU, N)))
    {
      mpLog::Success("Results for CPU (Up/Down Sweep) and GPU are equal!");
    }

    return Quit;
  }

  void GPU(cl_int* originalData)
  {
    MP_LogBlock("GPU");

    {
      MP_LogBlock("Initialization");
      Platform.Initialize();
      Device = mpDevice::GetGPU(Platform, 0);
      Context.Initialize(Device);
      Queue.Initialize(Context, Device);
      MP_Verify(Program.LoadAndBuild(Context, Device, "Kernels/PrefixSum.cl"));
      Kernel_PrefixSum.Initialize(Queue, Program, "PrefixSum");
      Kernel_PrefixSumHelper.Initialize(Queue, Program, "PrefixSumHelper");
      Kernel_PrefixSumReduce.Initialize(Queue, Program, "PrefixSumReduce");
    }

    const auto A = ((N + blockSize - 1) / blockSize) * blockSize;
    MP_Assert(A % blockSize == 0, "Odd block sizes are not allowed!");
    const auto numBlocks = A / blockSize;

    MP_Assert(A >= N, "A must be >= N!");

    cl_int* inputData;
    if (A == N)
    {
      inputData = originalData;
    }
    else
    {
      // Increase the `inputData` array to a multiple of blockSize and
      // initialize the new space with 0.
      inputData = new cl_int[A];
      memcpy(inputData, originalData, N * sizeof(cl_int));
      memset(inputData + N, 0, (A - N) * sizeof(cl_int));
    }

    mpLog::Info("Original N:     %u", N);
    mpLog::Info("Adjusted N (A): %u", A);

    mpBuffer bufferA;
    bufferA.Initialize(Context,
                       mpBufferFlags::ReadWrite,
                       mpMakeArrayPtr(inputData, A));

    mpBuffer bufferB;
    bufferB.Initialize(Context,
                       mpBufferFlags::ReadWrite,
                       A * sizeof(cl_int));

    mpLog::Info("Running...");
    PrefixSumGPU(bufferA, bufferB, blockSize, numBlocks);

    bufferB.ReadInto(mpMakeArrayPtr(outputData_GPU, A), Queue);
    PrintData(mpMakeArrayPtr(outputData_GPU, N));

    if (inputData != originalData)
    {
      delete[] inputData;
    }
  }

  /// \param temp Is of size \a blockSize.
  void PrefixSumGPU(mpBuffer& bufferA,
                    mpBuffer& bufferB,
                    cl_int blockSize,
                    cl_int numBlocks)
  {
    MP_Assert(numBlocks >= 1, "Need at least 1 block to work on.");

    MP_LogBlock("Prefix Sum");
    mpLog::Info("Number of blocks:            %u", numBlocks);
    mpLog::Info("Block Size:                  %u", blockSize);
    mpLog::Info("Total number of threads:     %u", numBlocks * blockSize / 2);
    mpLog::Info("Number of threads per block: %u", blockSize / 2);

    // Blockwise Prefix Sum
    // ====================
    Kernel_PrefixSum.PushArg(bufferA);
    Kernel_PrefixSum.PushArg(bufferB);
    Kernel_PrefixSum.PushArg(mpLocalMemory<cl_int>(blockSize * numBlocks));
    Kernel_PrefixSum.Execute(numBlocks * blockSize / 2, blockSize / 2);

    bufferB.ReadInto(mpMakeArrayPtr(outputData_GPU, numBlocks * blockSize), Queue);

    // If we only calculated 1 block, we are done.
    if (numBlocks == 1)
      return;

    auto tempLength = ((numBlocks + blockSize - 1) / blockSize) * blockSize;

    // Create `temp`
    // =============
    // Note: mpBuffer objects clean themselves up in their destructor.
    mpBuffer temp1;
    mpBuffer temp2;
    {
      // We must initialize temp with 0!
      auto tempContent = new cl_int[tempLength];
      memset(tempContent, 0, tempLength * sizeof(cl_int));

      temp1.Initialize(Context,
                       mpBufferFlags::ReadWrite,
                       mpMakeArrayPtr(tempContent, tempLength));
      temp2.Initialize(Context,
                       mpBufferFlags::ReadWrite,
                       mpMakeArrayPtr(tempContent, tempLength));

      delete[] tempContent;
    }

    // Populate `temp1` with the sums of each block
    // ===========================================
    Kernel_PrefixSumHelper.PushArg(bufferA);
    Kernel_PrefixSumHelper.PushArg(bufferB);
    Kernel_PrefixSumHelper.PushArg(temp1);
    Kernel_PrefixSumHelper.PushArg(blockSize);
    Kernel_PrefixSumHelper.Execute(numBlocks);

    temp1.ReadInto(mpMakeArrayPtr(outputData_GPU, tempLength), Queue);

    // Prefix sums of `temp`
    // =====================
    PrefixSumGPU(temp1, temp2, blockSize, tempLength / blockSize);

    temp2.ReadInto(mpMakeArrayPtr(outputData_GPU, tempLength), Queue);

    // Map all values in `temp` to blocks in `bufferB`
    // ===============================================
    Kernel_PrefixSumReduce.PushArg(bufferB);
    Kernel_PrefixSumReduce.PushArg(temp2);
    Kernel_PrefixSumReduce.Execute(numBlocks * blockSize / 2, blockSize / 2);

    bufferB.ReadInto(mpMakeArrayPtr(outputData_GPU, numBlocks * blockSize), Queue);
    PrintData(mpMakeArrayPtr(outputData_GPU, numBlocks * blockSize));
  }
};

MP_EntryClass(Main);
