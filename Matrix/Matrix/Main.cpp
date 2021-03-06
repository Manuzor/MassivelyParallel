#include "Matrix/PCH.h"
#include "mpWrapper/Utilities/Console.h"
#include "mpWrapper/Types/Matrix.h"
#include "mpWrapper/Utilities.h"
#include "mpWrapper/Foundation/Startup.h"

static void LogGeneralStats(mpTime CPUTime, mpTime GPUTime)
{
  const char* szMessage;
  mpTime Difference;
  double fFactor;

  if(CPUTime < GPUTime)
  {
    szMessage = "The CPU was faster than the GPU";
    Difference = GPUTime - CPUTime;
    fFactor = GPUTime.GetSeconds() / CPUTime.GetSeconds();
  }
  else
  {
    szMessage = "The GPU was faster than the CPU";
    Difference = CPUTime - GPUTime;
    fFactor = CPUTime.GetSeconds() / GPUTime.GetSeconds();
  }

  mpLog::Info("- CPU Time:  %f seconds", CPUTime.GetSeconds());
  mpLog::Info("- GPU Time:  %f seconds", GPUTime.GetSeconds());
  mpLog::Info("- Delta:     %f seconds", Difference.GetSeconds());
  mpLog::Info("- Factor:    %f", fFactor);
  mpLog::Info("%s", szMessage);
}

static void Test1()
{
  mpMatrix MatrixLeft(4, 2);
  MatrixLeft(0, 0) = 1;
  MatrixLeft(1, 0) = 2;
  MatrixLeft(2, 0) = 3;
  MatrixLeft(3, 0) = 4;
  MatrixLeft(0, 1) = 5;
  MatrixLeft(1, 1) = 6;
  MatrixLeft(2, 1) = 7;
  MatrixLeft(3, 1) = 8;

  mpMatrix MatrixRight(2, 4);
  MatrixRight(0, 0) = 1;
  MatrixRight(1, 0) = 2;
  MatrixRight(0, 1) = 3;
  MatrixRight(1, 1) = 4;
  MatrixRight(0, 2) = 5;
  MatrixRight(1, 2) = 6;
  MatrixRight(0, 3) = 7;
  MatrixRight(1, 3) = 8;

  auto MatrixResult = MatrixLeft * MatrixRight;
  MP_Assert(MatrixResult.GetHeight() == 4, "");
  MP_Assert(MatrixResult.GetWidth() == 4, "");

  MP_Assert((MatrixResult(0, 0) == 11), "Wrong Result");
  MP_Assert((MatrixResult(1, 0) == 14), "Wrong Result");
  MP_Assert((MatrixResult(2, 0) == 17), "Wrong Result");
  MP_Assert((MatrixResult(3, 0) == 20), "Wrong Result");
  MP_Assert((MatrixResult(0, 1) == 23), "Wrong Result");
  MP_Assert((MatrixResult(1, 1) == 30), "Wrong Result");
  MP_Assert((MatrixResult(2, 1) == 37), "Wrong Result");
  MP_Assert((MatrixResult(3, 1) == 44), "Wrong Result");
  MP_Assert((MatrixResult(0, 2) == 35), "Wrong Result");
  MP_Assert((MatrixResult(1, 2) == 46), "Wrong Result");
  MP_Assert((MatrixResult(2, 2) == 57), "Wrong Result");
  MP_Assert((MatrixResult(3, 2) == 68), "Wrong Result");
  MP_Assert((MatrixResult(0, 3) == 47), "Wrong Result");
  MP_Assert((MatrixResult(1, 3) == 62), "Wrong Result");
  MP_Assert((MatrixResult(2, 3) == 77), "Wrong Result");
  MP_Assert((MatrixResult(3, 3) == 92), "Wrong Result");

  mpLog::Success("Test1 completed.\n");
}

static void Test2()
{
  mpMatrix MatrixLeft(4, 4);
  size_t i(0);
  for(size_t c = 0; c < MatrixLeft.GetWidth(); c++)
    for(size_t r = 0; r < MatrixLeft.GetHeight(); r++)
      MatrixLeft(r, c) = (mpMatrix::ElementType)i++;

  mpMatrix MatrixRight(4, 4, Identity);

  auto MatrixResult = MatrixLeft * MatrixRight;

  for(size_t c = 0; c < MatrixLeft.GetWidth(); c++)
    for(size_t r = 0; r < MatrixLeft.GetHeight(); r++)
      MP_Assert((MatrixResult(r, c) == MatrixLeft(r, c)), "Identity matrix not working.");

  mpLog::Success("Test2 completed.\n");
}

static void Test3()
{
  mpPlatform Platform;
  Platform.Initialize();
  auto Device = mpDevice::GetGPU(Platform, 0);
  mpContext Context;
  Context.Initialize(Device);
  mpCommandQueue Commands;
  Commands.Initialize(Context, Device);
  mpProgram Program;
  MP_Verify(Program.LoadAndBuild(Context, Device, "Kernels/Matrix.cl"));
  mpKernel Kernel;
  Kernel.Initialize(Commands, Program, "MultiplyMatrix");

  {
    MP_GPUScope(Context, Commands, Kernel);
    mpMatrix Left(1, 1, Identity);
    mpMatrix Right(1, 1, Identity);
    auto Result = Left * Right;

    for(size_t c = 0; c < Left.GetWidth(); c++)
      for(size_t r = 0; r < Right.GetHeight(); r++)
        mpLog::Info("res: %f", Result(r, c));
  }

  mpLog::Success("Test3 completed.\n");
}

// Load matrix from file and multiply
static void Test4()
{
  auto Left = mpMatrix::FromFile("Data/4x2_123.matrix.txt");
  MP_Assert(Left.GetHeight() == 4, ""); MP_Assert(Left.GetWidth() == 2, "");

  auto Right = mpMatrix::FromFile("Data/2x4_123.matrix.txt");
  MP_Assert(Right.GetHeight() == 2, ""); MP_Assert(Right.GetWidth() == 4, "");

  auto Result = Left * Right;

  MP_Assert((Result(0, 0) == 11), "Wrong Result");
  MP_Assert((Result(1, 0) == 14), "Wrong Result");
  MP_Assert((Result(2, 0) == 17), "Wrong Result");
  MP_Assert((Result(3, 0) == 20), "Wrong Result");
  MP_Assert((Result(0, 1) == 23), "Wrong Result");
  MP_Assert((Result(1, 1) == 30), "Wrong Result");
  MP_Assert((Result(2, 1) == 37), "Wrong Result");
  MP_Assert((Result(3, 1) == 44), "Wrong Result");
  MP_Assert((Result(0, 2) == 35), "Wrong Result");
  MP_Assert((Result(1, 2) == 46), "Wrong Result");
  MP_Assert((Result(2, 2) == 57), "Wrong Result");
  MP_Assert((Result(3, 2) == 68), "Wrong Result");
  MP_Assert((Result(0, 3) == 47), "Wrong Result");
  MP_Assert((Result(1, 3) == 62), "Wrong Result");
  MP_Assert((Result(2, 3) == 77), "Wrong Result");
  MP_Assert((Result(3, 3) == 92), "Wrong Result");

  mpLog::Success("Test4 completed.\n");
}

static void Test5(const char* szFileName, const char* szKernelFile)
{
  MP_LogBlock("Test5");
  mpLog::Info("Matrix: %s", szFileName);
  mpLog::Info("Kernel: %s", szKernelFile);

  mpPlatform Platform;
  Platform.Initialize();
  auto Device = mpDevice::GetGPU(Platform, 0);
  mpContext Context;
  Context.Initialize(Device);
  mpCommandQueue Commands;
  Commands.Initialize(Context, Device);
  mpProgram Program;
  MP_Verify(Program.LoadAndBuild(Context, Device, szKernelFile));
  mpKernel Kernel;
  Kernel.Initialize(Commands, Program, "MultiplyMatrix");

  //////////////////////////////////////////////////////////////////////////

  mpLog::Info("Loading matrix data from file '%s'", szFileName);
  auto DataLoadingBegin = mpTime::Now();
  auto Left = mpMatrix::FromFile(szFileName);
  mpLog::Info("Loaded a %ux%u matrix in %f seconds.",
              Left.GetHeight(), Left.GetWidth(),
              (mpTime::Now() - DataLoadingBegin).GetSeconds());

  //////////////////////////////////////////////////////////////////////////

  auto Right = Left;

  decltype(Left * Right) CPUResult(NoInit);
  decltype(Left * Right) GPUResult(NoInit);

  mpTime CPUTime;
  mpTime GPUTime;

  {
    MP_LogBlock("CPU");
    mpLog::Info("Calculating...");
    auto Beginning = mpTime::Now();
    CPUResult = std::move(Left * Right);
    CPUTime = mpTime::Now() - Beginning;
    mpLog::Success("Finished in %f seconds.", CPUTime);
  }

  {
    MP_LogBlock("GPU");
    MP_GPUScope(Context, Commands, Kernel);

    mpLog::Info("Calculating...");
    auto Beginning = mpTime::Now();
    GPUResult = std::move(Left * Right);
    GPUTime = mpTime::Now() - Beginning;
    mpLog::Success("Finished in %f seconds.", GPUTime);
  }

  {
    auto comparisonEpsilon = 1e-4f;
    MP_LogBlock("Comparing CPU and GPU results");
    mpLog::Info("Epsilon = %f", comparisonEpsilon);

    MP_Assert(GPUResult.GetHeight() == CPUResult.GetHeight(), "Invalid height");
    MP_Assert(GPUResult.GetWidth() == CPUResult.GetWidth(), "Invalid width");

    for(size_t c = 0; c < CPUResult.GetWidth(); c++)
    {
      for(size_t r = 0; r < CPUResult.GetHeight(); r++)
      {
        auto gpu = GPUResult(r, c);
        auto cpu = CPUResult(r, c);
        MP_Assert(mpMath::IsEqual(gpu, cpu, comparisonEpsilon), "Invalid result.");
      }
    }

    mpLog::Success("The results are equal!");

    {
      MP_LogBlock("Statistics");
      mpLog::Info("L Matrix:  %ux%u", Left.GetHeight(), Left.GetWidth());
      mpLog::Info("Operation: Multiplication");
      LogGeneralStats(CPUTime, GPUTime);
    }
  }
}

template<typename MatrixType>
static mpTime Test6Helper(MatrixType& lhs, MatrixType& rhs)
{
  auto Beginning = mpTime::Now();
  auto Result = lhs * rhs;
  return mpTime::Now() - Beginning;
}

static void Test6(size_t uiHeight, size_t uiWidth, const char* szKernelFile)
{
  MP_LogBlock("Test6");
  mpLog::Info("Create two randomly generated %ux%u matrices and multiply them "
              "on the CPU and on the GPU using the kernel %s",
              uiHeight, uiWidth, szKernelFile);

  mpLog::Info("Creating matrices . . .");
  auto Beginning = mpTime::Now();
  auto Left  = mpMatrix::Random(uiHeight, uiWidth, { -1.0f, 1.0f });
  auto Right = mpMatrix::Random(uiHeight, uiWidth, { -1.0f, 1.0f });

  mpLog::Success("Created matrices in %f seconds", (mpTime::Now() - Beginning).GetSeconds());

  const size_t uiIterations = 1;
  mpTime CPUTime;
  mpTime GPUTime;

  {
    MP_LogBlock("CPU");
    mpLog::Info("Calulcating matrix-matrix product %u times . . .", uiIterations);
    for(size_t i = 0; i < uiIterations; i++)
    {
      mpLog::Info("Iteration #%u", i + 1);
      CPUTime += Test6Helper(Left, Right);
    }
    mpLog::Success("Finished in %f seconds => %f seconds per iteration",
                   CPUTime.GetSeconds(), CPUTime.GetSeconds() / uiIterations);
  }

  //
  //////////////////////////////////////////////////////////////////////////
  {
    MP_LogBlock("GPU");

    mpPlatform Platform;
    Platform.Initialize();
    auto Device = mpDevice::GetGPU(Platform, 0);
    mpContext Context;
    mpCommandQueue Commands;
    mpProgram Program;
    mpKernel Kernel;

    {
      MP_LogBlock("Setup");
      MP_LogLevelForScope(mpLogLevel::None);

      Context.Initialize(Device);
      Commands.Initialize(Context, Device);
      MP_Verify(Program.LoadAndBuild(Context, Device, szKernelFile));
      Kernel.Initialize(Commands, Program, "MultiplyMatrix");
    }

    MP_GPUScope(Context, Commands, Kernel);
    mpLog::Info("Calulcating matrix-matrix product %u times . . .", uiIterations);
    for(size_t i = 0; i < uiIterations; i++)
    {
      mpLog::Info("Iteration #%u", i + 1);
      GPUTime += Test6Helper(Left, Right);
    }
    mpLog::Success("Finished in %f seconds => %f seconds per iteration",
                   GPUTime.GetSeconds(), GPUTime.GetSeconds() / uiIterations);
  }

  {
    MP_LogBlock("Statistics");
    mpLog::Info("- L Matrix:  %ux%u", uiHeight, uiWidth);
    mpLog::Info("- R Matrix:  %ux%u", uiHeight, uiWidth);
    mpLog::Info("- Operation: Multiplication");
    LogGeneralStats(CPUTime, GPUTime);
  }
}

class Main : public mpApplication
{
  mpTime m_Beginning;

  virtual void PreStartup() final override
  {
    m_Beginning = mpTime::Now();
  }

  virtual QuitOrContinue Run() final override
  {
    //Test1();
    //Test2();
    //Test3();
    //Test4();
    Test5("Data/HugeRandomMatrix.txt", "Kernels/Matrix.cl");

    size_t uiFrom = 256;
    size_t uiTo = 1024; // Inclusive
    size_t uiIterations = 0;
    for (size_t uiDim = uiFrom; uiDim < uiTo + 1; uiDim <<= 1)
    {
      ++uiIterations;
      MP_LogBlock("#%u:", uiIterations);
      Test6(uiDim, uiDim, "Kernels/Matrix.cl");
    }

    return Quit;
  }

  virtual void PostShutdown() final override
  {
    mpLog::Info("The application ran for %f seconds.",
                (mpTime::Now() - m_Beginning).GetSeconds());
    printf("\nPress any key to quit . . . ");
    mpUtilities::GetSingleCharacter();
  }
};

MP_EntryClass(Main)
