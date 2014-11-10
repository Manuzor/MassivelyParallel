#pragma once

class mpContext;
class mpCommandQueue;
class mpKernel;

struct MP_WrapperAPI mpGPUScope
{
  static mpGPUScope* GetCurrentScope();

  ~mpGPUScope();

  mpGPUScope(mpContext& Context, mpCommandQueue& CommandQueue, mpKernel& Kernel);

  MP_ForceInline mpContext& GetContext() { MP_Assert(m_pContext, ""); return *m_pContext; }
  MP_ForceInline mpCommandQueue& GetCommandQueue() { MP_Assert(m_pCommandQueue, ""); return *m_pCommandQueue; }
  MP_ForceInline mpKernel& GetKernel() { MP_Assert(m_pKernel, ""); return *m_pKernel; }

private:
  mpGPUScope* m_pPreviousScope;

  mpContext* m_pContext;
  mpCommandQueue* m_pCommandQueue;
  mpKernel* m_pKernel;
};

#define MP_GPUScope mpGPUScope MP_Concat(_gpuScope_, __LINE__)
