#include "mpWrapper/PCH.h"
#include "mpWrapper/Core/Buffer.h"
#include "mpWrapper/Core/CommandQueue.h"

void mpBuffer::Initialize(const mpContext& Context, mpBufferFlags Flags, size_t uiBufferSize, void* RawBuffer)
{
  Release();

  cl_int status;
  if (RawBuffer != nullptr)
    m_Id = clCreateBuffer(Context.m_Id, Flags.m_Value | CL_MEM_COPY_HOST_PTR, uiBufferSize, RawBuffer, &status);
  else
    m_Id = clCreateBuffer(Context.m_Id, Flags.m_Value , uiBufferSize, nullptr, &status);
  MP_Verify(status);
}

void mpBuffer::Release()
{
  if (m_Id == nullptr)
    return;

  MP_Verify(clReleaseMemObject(m_Id));
  m_Id = nullptr;
}

void mpBuffer::ReadInto(size_t uiBufferSize, void* Buffer, const mpCommandQueue& Queue)
{
  auto status = clEnqueueReadBuffer(Queue.m_Id, m_Id,     // Queue and Buffer
                                    CL_TRUE,              // Blocking read
                                    0,                    // Read-offset
                                    uiBufferSize, Buffer, // Buffer size and the actual buffer
                                    0, nullptr, nullptr); // Event stuff.
  MP_Verify(status);
}
