#include "mpWrapper/PCH.h"
#include "mpWrapper/Core/Scope.h"

static size_t g_uiGPUScopeCount = 0;
static mpGPUScope* g_pCurrentScope = nullptr;

mpGPUScope* mpGPUScope::GetCurrentScope()
{
  return g_pCurrentScope;
}

mpGPUScope::mpGPUScope(mpContext& Context, mpCommandQueue& CommandQueue, mpKernel& Kernel) :
  m_pPreviousScope(g_pCurrentScope),
  m_pContext(&Context),
  m_pCommandQueue(&CommandQueue),
  m_pKernel(&Kernel)
{
  g_pCurrentScope = this;
}

mpGPUScope::~mpGPUScope()
{
  g_pCurrentScope = m_pPreviousScope;
  m_pPreviousScope = nullptr;

  m_pContext = nullptr;
  m_pCommandQueue = nullptr;
  m_pKernel = nullptr;
}
