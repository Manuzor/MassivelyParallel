#include "mpWrapper/PCH.h"
#include "mpWrapper/Core/Platform.h"

void mpPlatform::Initialize()
{
  if (m_Id != nullptr)
    Shutdown();

  cl_uint numPlatforms; // the number of platforms
  MP_Verify(clGetPlatformIDs(0, NULL, &numPlatforms));

  /*For clarity, choose the first available platform. */
  if(numPlatforms > 0)
  {
    cl_platform_id* platforms = (cl_platform_id* )alloca(numPlatforms * sizeof(cl_platform_id));
    MP_Verify(clGetPlatformIDs(numPlatforms, platforms, NULL));
    m_Id = platforms[0];
  }
  else
  {
    MP_ReportError("There are no available OpenCL platforms!");
  }
}

void mpPlatform::Shutdown()
{
  if (m_Id == nullptr)
    return;

  // TODO: Free stuff here?

  m_Id = nullptr;
}
