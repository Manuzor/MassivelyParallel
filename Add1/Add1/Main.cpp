#include "Add1/PCH.h"
#include "mpWrapper/Utilities/Console.h"

int main(int argc, char* argv[])
{
  auto Platform = mpPlatform::Get();
  auto Device = mpDevice::GetGPU(Platform, 0);
  mpContext Context;
  Context.Initialize(Device);
  mpCommandQueue Commands;
  Commands.Initialize(Context, Device);
  mpProgram Program;
  MP_Verify(Program.LoadAndBuild(Context, Device, "Kernels/Add1.cl"));
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
