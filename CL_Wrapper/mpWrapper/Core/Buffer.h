#pragma once
#include "mpWrapper/Core/Context.h"

class mpCommandQueue;

struct MP_WrapperAPI mpBufferFlags
{
  enum Bits : unsigned char
  {
    ReadWrite = CL_MEM_READ_WRITE,
    ReadOnly  = CL_MEM_READ_ONLY,
    WriteOnly = CL_MEM_WRITE_ONLY,

    UseHostPtr   = CL_MEM_USE_HOST_PTR,
    AllocHostPtr = CL_MEM_ALLOC_HOST_PTR,
    CopyHostPtr  = CL_MEM_COPY_HOST_PTR,
  };

  mpBufferFlags(unsigned char Value) : m_Value(Value) {}

  unsigned char m_Value;
};

class MP_WrapperAPI mpBuffer
{
public:
  cl_mem m_Id = nullptr;

  ~mpBuffer(){ Release(); }

  template<typename Type>
  void Initialize(const mpContext& Context, mpBufferFlags Flags, mpArrayPtr<Type> Buffer)
  {
    Initialize(Context, Flags, Buffer.m_uiCount * sizeof(Type), Buffer.m_Data);
  }

  void Initialize(const mpContext& Context, mpBufferFlags Flags, size_t uiBufferSize)
  {
    Initialize(Context, Flags, uiBufferSize, nullptr);
  }

  void Release();

  template<typename Type>
  void ReadInto(mpArrayPtr<Type> OutBuffer, const mpCommandQueue& Queue)
  {
    ReadInto(OutBuffer.m_uiCount * sizeof(Type), OutBuffer.m_Data, Queue);
  }

  template<typename VaueType>
  void ReadInto(VaueType& OutValue, const mpCommandQueue& Queue)
  {
    ReadInto(sizeof(VaueType), &OutValue, Queue);
  }

private:
  void Initialize(const mpContext& Context, mpBufferFlags Flags, size_t uiBufferSize, void* RawBuffer);
  void ReadInto(size_t uiBufferSize, void* Buffer, const mpCommandQueue& Queue);
};
