#include "mpWrapper/PCH.h"
#include "mpWrapper/Core/Context.h"
#include "mpWrapper/Core/Device.h"

void mpContext::Initialize(const mpDevice& Device)
{
  Release();

  cl_int status;
  m_Id = clCreateContext(nullptr, 1, &Device.m_Id, nullptr, nullptr, &status);
  MP_Verify(status);
}

void mpContext::Release()
{
  if (m_Id == nullptr)
    return;

  MP_Verify(clReleaseContext(m_Id));
  m_Id = nullptr;
}
