#include "Matrix/PCH.h"
#include "Wrapper/Utilities/Console.h"

int main(int argc, char* argv[])
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
  Kernel.Initialize(Program, "add1");

  mpLog::Info("Press any key to continue . . . ");
  mpUtilities::GetSingleCharacter();
  return 0;
}
