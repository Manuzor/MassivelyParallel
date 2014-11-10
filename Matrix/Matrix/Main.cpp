#include "Matrix/PCH.h"
#include "Wrapper/Utilities/Console.h"
#include "Wrapper/Types/Matrix.h"

static void Test1()
{
  mpMatrix<4, 2> MatrixLeft;
  MatrixLeft.At<0, 0>() = 1;
  MatrixLeft.At<1, 0>() = 2;
  MatrixLeft.At<2, 0>() = 3;
  MatrixLeft.At<3, 0>() = 4;
  MatrixLeft.At<0, 1>() = 5;
  MatrixLeft.At<1, 1>() = 6;
  MatrixLeft.At<2, 1>() = 7;
  MatrixLeft.At<3, 1>() = 8;

  mpMatrix<2, 4> MatrixRight;
  MatrixRight.At<0, 0>() = 1;
  MatrixRight.At<1, 0>() = 2;
  MatrixRight.At<0, 1>() = 3;
  MatrixRight.At<1, 1>() = 4;
  MatrixRight.At<0, 2>() = 5;
  MatrixRight.At<1, 2>() = 6;
  MatrixRight.At<0, 3>() = 7;
  MatrixRight.At<1, 3>() = 8;

  auto MatrixResult = MatrixLeft * MatrixRight;
  static_assert(decltype(MatrixResult)::Width == 4, "");
  static_assert(decltype(MatrixResult)::Height == 4, "");

  MP_Assert((MatrixResult.At<0, 0>() == 11), "Wrong Result");
  MP_Assert((MatrixResult.At<1, 0>() == 14), "Wrong Result");
  MP_Assert((MatrixResult.At<2, 0>() == 17), "Wrong Result");
  MP_Assert((MatrixResult.At<3, 0>() == 20), "Wrong Result");
  MP_Assert((MatrixResult.At<0, 1>() == 23), "Wrong Result");
  MP_Assert((MatrixResult.At<1, 1>() == 30), "Wrong Result");
  MP_Assert((MatrixResult.At<2, 1>() == 37), "Wrong Result");
  MP_Assert((MatrixResult.At<3, 1>() == 44), "Wrong Result");
  MP_Assert((MatrixResult.At<0, 2>() == 35), "Wrong Result");
  MP_Assert((MatrixResult.At<1, 2>() == 46), "Wrong Result");
  MP_Assert((MatrixResult.At<2, 2>() == 57), "Wrong Result");
  MP_Assert((MatrixResult.At<3, 2>() == 68), "Wrong Result");
  MP_Assert((MatrixResult.At<0, 3>() == 47), "Wrong Result");
  MP_Assert((MatrixResult.At<1, 3>() == 62), "Wrong Result");
  MP_Assert((MatrixResult.At<2, 3>() == 77), "Wrong Result");
  MP_Assert((MatrixResult.At<3, 3>() == 92), "Wrong Result");
}

static void Test2()
{
  mpMatrix<4, 4> MatrixLeft;
  size_t i(0);
  for(size_t c = 0; c < MatrixLeft.Width; c++)
    for(size_t r = 0; r < MatrixLeft.Height; r++)
      MatrixLeft.At(r, c) = (mpMatrix<4, 4>::ElementType)i++;

  mpMatrix<4, 4> MatrixRight(Identity);

  auto MatrixResult = MatrixLeft * MatrixRight;

  for(size_t c = 0; c < MatrixLeft.Width; c++)
    for(size_t r = 0; r < MatrixLeft.Height; r++)
      MP_Assert((MatrixResult.At(r, c) == MatrixLeft.At(r, c)), "Identity matrix not working.");
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
    mpMatrix<1, 1> Left;  Left.At<0, 0>() = 1.0f;
    mpMatrix<1, 1> Right; Right.At<0, 0>() = 1.0f;
    auto Result = Left * Right;

    for(size_t c = 0; c < Left.Width; c++)
      for(size_t r = 0; r < Right.Height; r++)
        mpLog::Info("res: %f", Result.At(r, c));
  }
}

int main(int argc, char* argv[])
{
  auto LoadResult = mpMatrix<2, 4>::FromFile("Data/test.matrix.txt");

  Test1();
  Test2();
  Test3();

  printf("Press any key to continue . . . ");
  mpUtilities::GetSingleCharacter();
  return 0;
}
