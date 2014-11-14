#include "Matrix/PCH.h"
#include "Wrapper/Utilities/Console.h"
#include "Wrapper/Types/Matrix.h"
#include "Wrapper/Utilities/Math.h"
#include "Wrapper/Utilities/Time.h"
#include "Wrapper/Startup.h"

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
  auto Platform = mpPlatform::Get();
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
  auto Platform = mpPlatform::Get();
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

  mpLog::Info("Calculating result on the CPU...");
  auto Beginning = mpTime::Now();
  auto CPUResult = Left * Right;
  auto CPUTime = mpTime::Now() - Beginning;
  mpLog::Success("CPU calculation done in %f seconds.", CPUTime);

  {
    MP_GPUScope(Context, Commands, Kernel);

    mpLog::Info("Calculating result on the GPU...");
    auto Beginning = mpTime::Now();
    auto GPUResult = Left * Right;
    auto GPUTime = mpTime::Now() - Beginning;
    mpLog::Success("GPU calculation done in %f seconds.", GPUTime);

    {
      auto CPUSeconds = CPUTime.GetSeconds();
      auto GPUSeconds = GPUTime.GetSeconds();
      if(CPUSeconds < GPUSeconds)
      {
        auto Difference = GPUSeconds - CPUSeconds;
        mpLog::Info("The CPU was faster than the GPU by %.1f times (%f seconds).",
                    GPUSeconds / CPUSeconds,
                    Difference);
      }
      else
      {
        auto Difference = CPUSeconds - GPUSeconds;
        mpLog::Info("The GPU was faster than the CPU by %.1f times (%f seconds).",
                    CPUSeconds / GPUSeconds,
                    Difference);
      }
    }

    auto comparisonEpsilon = 0.9f;
    mpLog::Info("Comparing CPU and GPU results with an epsilon of %f", comparisonEpsilon);

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
  }

  mpLog::Success("Test5 completed.\n");
}


class Main : public mpApplication
{
  virtual RunBehavior Run() final override
  {
    auto Beginning = mpTime::Now();
    //Test1();
    //Test2();
    //Test3();
    //Test4();
    //Test5("Data/HugeRandomMatrix.txt", "Kernels/Matrix.cl");

    {
      MP_LogBlock("Level 0");
      mpLog::Info("A");
      {
        MP_LogBlock("Level 1");
        mpLog::Info("B");
        {
          MP_LogBlock("Level 2");
          mpLog::Info("C");
        }
        mpLog::Info("D");
      }
      mpLog::Info("E");
    }

    mpLog::Info("Needed %f seconds in total.", mpTime::Now() - Beginning);

    printf("\nPress any key to quit . . . ");
    mpUtilities::GetSingleCharacter();
    return RunBehavior::Quit;
  }
};

MP_EntryClass(Main)
