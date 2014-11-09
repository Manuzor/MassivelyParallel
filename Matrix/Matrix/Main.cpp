#include "Matrix/PCH.h"
#include "Wrapper/Utilities/Console.h"
#include "Matrix/Matrix.h"

int main(int argc, char* argv[])
{
  Matrix<4, 2> MatrixLeft;
  MatrixLeft.At<0, 0>() = 1;
  MatrixLeft.At<1, 0>() = 2;
  MatrixLeft.At<2, 0>() = 3;
  MatrixLeft.At<3, 0>() = 4;
  MatrixLeft.At<0, 1>() = 5;
  MatrixLeft.At<1, 1>() = 6;
  MatrixLeft.At<2, 1>() = 7;
  MatrixLeft.At<3, 1>() = 8;

  Matrix<2, 4> MatrixRight;
  MatrixRight.At<0, 0>() = 1;
  MatrixRight.At<1, 0>() = 2;
  MatrixRight.At<0, 1>() = 3;
  MatrixRight.At<1, 1>() = 4;
  MatrixRight.At<0, 2>() = 5;
  MatrixRight.At<1, 2>() = 6;
  MatrixRight.At<0, 3>() = 7;
  MatrixRight.At<1, 3>() = 8;

  auto MatrixResult = MatrixLeft * MatrixRight;
  MP_Assert((MatrixResult.At<0, 0>() == 11), "Wrong Result");
  MP_Assert((MatrixResult.At<1, 0>() == 14), "Wrong Result");
  MP_Assert((MatrixResult.At<2, 0>() == 17), "Wrong Result");
  MP_Assert((MatrixResult.At<3, 0>() == 20), "Wrong Result");
  MP_Assert((MatrixResult.At<0, 1>() == 24), "Wrong Result");
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

  auto Platform = mpPlatform::Get();
  auto Device = mpDevice::GetGPU(Platform, 0);
  mpContext Context;
  Context.Initialize(Device);
  mpCommandQueue Commands;
  Commands.Initialize(Context, Device);
  mpProgram Program;
  MP_Verify(Program.LoadAndBuild(Context, Device, "Kernels/Matrix.cl"));
  mpKernel Kernel;
  Kernel.Initialize(Commands, Program, "add1");

  const size_t uiIntegersCount = 8;
  cl_int Integers[uiIntegersCount];

  for (size_t i = 0; i < uiIntegersCount; ++i)
  {
    Integers[i] = (cl_int)i;
    mpLog::Info("in : %u", i);
  }

  mpBuffer IntegerBuffer;
  IntegerBuffer.Initialize(Context, mpBufferFlags::ReadOnly, mpMakeArrayPtr(Integers));

  mpBuffer Output;
  Output.Initialize(Context, mpBufferFlags::WriteOnly, uiIntegersCount * sizeof(cl_int));

  Kernel.PushArg(IntegerBuffer);
  Kernel.PushArg(Output);

  Kernel.Execute(8, 1);

  cl_int Result[uiIntegersCount];

  // Blocks until the kernel finished executing.
  Output.ReadInto(mpMakeArrayPtr(Result), Commands);
  mpLog::Info("");

  for (size_t i = 0; i < uiIntegersCount; ++i)
  {
    mpLog::Info("out: %u", Result[i]);
  }

  printf("Press any key to continue . . . ");
  mpUtilities::GetSingleCharacter();
  return 0;
}
