#pragma once
#include "mpWrapper/Program.h"

class mpBuffer;
class mpCommandQueue;

class MP_WrapperAPI mpKernel
{
public:
  using Id = cl_kernel;

  Id m_Id = nullptr;
public:

  ~mpKernel() { Release(); }

  void Initialize(const mpCommandQueue& Queue, const mpProgram& Program, const char* szKernelName);
  void Release();
  bool IsInitialized() const { return m_Id != nullptr; }

  void PushArg(const mpBuffer& Buffer);

  template<typename ValueType>
  void PushArg(const ValueType& Value)
  {
    MP_Verify(clSetKernelArg(m_Id, m_uiCurrentArgCount, sizeof(ValueType), (const void*)&Value));
    ++m_uiCurrentArgCount;
  }

  void Execute(size_t uiGlobalWorkSize, size_t uiLocalWorkSize = 0)
  {
    size_t G[] { uiGlobalWorkSize };
    if (uiLocalWorkSize > 0)
    {
      size_t L[] { uiLocalWorkSize };
      Execute(G, L);
    }
    else
    {
      Execute(G, nullptr);
    }
  }

  void Execute(mpArrayPtr<size_t> GlobalWorkSize, mpArrayPtr<size_t> LocalWorkSize = nullptr);

private:
  cl_uint m_uiCurrentArgCount = 0;
  const mpCommandQueue* m_pQueue = nullptr;
};
