#include "Wrapper/PCH.h"
#include "Wrapper/Platform.h"

static cl_platform_id g_Platforms;

mpPlatform::Handle mpPlatform::Get()
{
  cl_uint numPlatforms; //the NO. of platforms
  cl_platform_id platform = NULL; //the chosen platform
  MP_Verify(clGetPlatformIDs(0, NULL, &numPlatforms));

  /*For clarity, choose the first available platform. */
  if(numPlatforms > 0)
  {
    cl_platform_id* platforms = (cl_platform_id* )alloca(numPlatforms * sizeof(cl_platform_id));
    MP_Verify(clGetPlatformIDs(numPlatforms, platforms, NULL));
    platform = platforms[0];
  }
  else
  {
    MP_ReportError("There are no available OpenCL platforms!");
  }

  return Handle(platform);
}
