#include "Wrapper/PCH.h"
#include "Wrapper/Kernel.h"


void mpKernel::Initialize(const mpProgram& Program, const char* szKernelName)
{
  Release();

  cl_int status;
  m_Id = clCreateKernel(Program.m_Id, szKernelName, &status);
  MP_Verify(status);
}

void mpKernel::Release()
{
  if (m_Id == nullptr)
    return;

  MP_Verify(clReleaseKernel(m_Id));
  m_Id = nullptr;
}
