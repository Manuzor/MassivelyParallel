#include "Wrapper/PCH.h"
#include "Wrapper/Kernel.h"
#include "Wrapper/Buffer.h"
#include "Wrapper/CommandQueue.h"

void mpKernel::Initialize(const mpCommandQueue& Queue, const mpProgram& Program, const char* szKernelName)
{
  Release();

  m_pQueue = &Queue;

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
  m_pQueue = nullptr;
}

void mpKernel::PushArg(const mpBuffer& Buffer)
{
  MP_Verify(clSetKernelArg(m_Id, m_uiCurrentArgCount, sizeof(decltype(Buffer.m_Id)), (const void*)&Buffer.m_Id));
  ++m_uiCurrentArgCount;
}

void mpKernel::Execute(mpArrayPtr<size_t> GlobalWorkSize, mpArrayPtr<size_t> LocalWorkSize)
{
  MP_Assert(LocalWorkSize.m_Data == nullptr || GlobalWorkSize.m_uiCount == LocalWorkSize.m_uiCount,
            "Global and Local work sizes must have the same dimensions");

  MP_Verify(clEnqueueNDRangeKernel(m_pQueue->m_Id, m_Id,                        // Command queue and kernel
                                   (cl_uint)GlobalWorkSize.m_uiCount, nullptr,  // Work dimensions and global offset
                                   GlobalWorkSize.m_Data, LocalWorkSize.m_Data, // Global and local work sizes
                                   0, nullptr, nullptr));                       // Event stuff

  m_uiCurrentArgCount = 0;
}
