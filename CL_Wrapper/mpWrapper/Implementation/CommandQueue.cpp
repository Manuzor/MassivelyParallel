#include "mpWrapper/PCH.h"
#include "mpWrapper/CommandQueue.h"

void mpCommandQueue::Initialize(const mpContext& Context, const mpDevice& Device)
{
  Release();

  cl_int status;
  m_Id = clCreateCommandQueue(Context.m_Id, Device.m_Id, 0, &status);
  MP_Verify(status);
}

void mpCommandQueue::Release()
{
  if(m_Id == nullptr)
    return;

  MP_Verify(clReleaseCommandQueue(m_Id));
  m_Id = nullptr;
}
