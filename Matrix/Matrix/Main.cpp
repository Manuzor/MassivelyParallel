#include "Matrix/PCH.h"
#include "Wrapper/Utilities/Console.h"
#include "Wrapper/Types/Matrix.h"

static void Test1()
{
  mpMatrix MatrixLeft(4, 2);
  MatrixLeft.At(0, 0) = 1;
  MatrixLeft.At(1, 0) = 2;
  MatrixLeft.At(2, 0) = 3;
  MatrixLeft.At(3, 0) = 4;
  MatrixLeft.At(0, 1) = 5;
  MatrixLeft.At(1, 1) = 6;
  MatrixLeft.At(2, 1) = 7;
  MatrixLeft.At(3, 1) = 8;

  mpMatrix MatrixRight(2, 4);
  MatrixRight.At(0, 0) = 1;
  MatrixRight.At(1, 0) = 2;
  MatrixRight.At(0, 1) = 3;
  MatrixRight.At(1, 1) = 4;
  MatrixRight.At(0, 2) = 5;
  MatrixRight.At(1, 2) = 6;
  MatrixRight.At(0, 3) = 7;
  MatrixRight.At(1, 3) = 8;

  auto MatrixResult = MatrixLeft * MatrixRight;
  MP_Assert(MatrixResult.GetHeight() == 4, "");
  MP_Assert(MatrixResult.GetWidth() == 4, "");

  MP_Assert((MatrixResult.At(0, 0) == 11), "Wrong Result");
  MP_Assert((MatrixResult.At(1, 0) == 14), "Wrong Result");
  MP_Assert((MatrixResult.At(2, 0) == 17), "Wrong Result");
  MP_Assert((MatrixResult.At(3, 0) == 20), "Wrong Result");
  MP_Assert((MatrixResult.At(0, 1) == 23), "Wrong Result");
  MP_Assert((MatrixResult.At(1, 1) == 30), "Wrong Result");
  MP_Assert((MatrixResult.At(2, 1) == 37), "Wrong Result");
  MP_Assert((MatrixResult.At(3, 1) == 44), "Wrong Result");
  MP_Assert((MatrixResult.At(0, 2) == 35), "Wrong Result");
  MP_Assert((MatrixResult.At(1, 2) == 46), "Wrong Result");
  MP_Assert((MatrixResult.At(2, 2) == 57), "Wrong Result");
  MP_Assert((MatrixResult.At(3, 2) == 68), "Wrong Result");
  MP_Assert((MatrixResult.At(0, 3) == 47), "Wrong Result");
  MP_Assert((MatrixResult.At(1, 3) == 62), "Wrong Result");
  MP_Assert((MatrixResult.At(2, 3) == 77), "Wrong Result");
  MP_Assert((MatrixResult.At(3, 3) == 92), "Wrong Result");

  mpLog::Success("Test1 completed.");
}

static void Test2()
{
  mpMatrix MatrixLeft(4, 4);
  size_t i(0);
  for(size_t c = 0; c < MatrixLeft.GetWidth(); c++)
    for(size_t r = 0; r < MatrixLeft.GetHeight(); r++)
      MatrixLeft.At(r, c) = (mpMatrix::ElementType)i++;

  mpMatrix MatrixRight(4, 4, Identity);

  auto MatrixResult = MatrixLeft * MatrixRight;

  for(size_t c = 0; c < MatrixLeft.GetWidth(); c++)
    for(size_t r = 0; r < MatrixLeft.GetHeight(); r++)
      MP_Assert((MatrixResult.At(r, c) == MatrixLeft.At(r, c)), "Identity matrix not working.");

  mpLog::Success("Test2 completed.");
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
  Kernel.Initialize(Commands, Program, "multiply");

  {
    MP_GPUScope(Context, Commands, Kernel);
    mpMatrix Left(1, 1, Identity);
    mpMatrix Right(1, 1, Identity);
    auto Result = Left * Right;

    for(size_t c = 0; c < Left.GetWidth(); c++)
      for(size_t r = 0; r < Right.GetHeight(); r++)
        mpLog::Info("res: %f", Result.At(r, c));
  }

  mpLog::Success("Test3 completed.");
}

// Load matrix from file and multiply
static void Test4()
{
  auto Left = mpMatrix::FromFile("Data/4x2_123.matrix.txt");
  MP_Assert(Left.GetHeight() == 4, ""); MP_Assert(Left.GetWidth() == 2, "");

  auto Right = mpMatrix::FromFile("Data/2x4_123.matrix.txt");
  MP_Assert(Right.GetHeight() == 2, ""); MP_Assert(Right.GetWidth() == 4, "");

  auto Result = Left * Right;

  MP_Assert((Result.At(0, 0) == 11), "Wrong Result");
  MP_Assert((Result.At(1, 0) == 14), "Wrong Result");
  MP_Assert((Result.At(2, 0) == 17), "Wrong Result");
  MP_Assert((Result.At(3, 0) == 20), "Wrong Result");
  MP_Assert((Result.At(0, 1) == 23), "Wrong Result");
  MP_Assert((Result.At(1, 1) == 30), "Wrong Result");
  MP_Assert((Result.At(2, 1) == 37), "Wrong Result");
  MP_Assert((Result.At(3, 1) == 44), "Wrong Result");
  MP_Assert((Result.At(0, 2) == 35), "Wrong Result");
  MP_Assert((Result.At(1, 2) == 46), "Wrong Result");
  MP_Assert((Result.At(2, 2) == 57), "Wrong Result");
  MP_Assert((Result.At(3, 2) == 68), "Wrong Result");
  MP_Assert((Result.At(0, 3) == 47), "Wrong Result");
  MP_Assert((Result.At(1, 3) == 62), "Wrong Result");
  MP_Assert((Result.At(2, 3) == 77), "Wrong Result");
  MP_Assert((Result.At(3, 3) == 92), "Wrong Result");

  mpLog::Success("Test4 completed.");
}

static void Test5()
{
  auto Left = mpMatrix::FromFile("Data/16x16_random.matrix.txt");
  MP_Assert(Left.GetHeight() == 16 && Left.GetWidth() == 16, "Invalid result.");

  auto Right = Left;
  auto Result = Left * Right;

  mpLog::Success("Test5 completed.");
}

int main(int argc, char* argv[])
{
  Test1();
  Test2();
  Test3();
  Test4();
  Test5();

  printf("\nPress any key to quit . . . ");
  mpUtilities::GetSingleCharacter();
  return 0;
}
