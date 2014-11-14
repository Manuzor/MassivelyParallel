#pragma once

#ifdef MP_Wrapper_DllExport
#define MP_WrapperAPI __declspec(dllexport)
#else
#define MP_WrapperAPI __declspec(dllimport)
#endif

#define MP_Inline inline
#define MP_ForceInline __forceinline

#include "mpWrapper/TypeDeclarations.h"

namespace mpInternal
{
  template<typename CallableType>
  struct OnScopeExit
  {
    CallableType m_ToCall;

    OnScopeExit(CallableType toCall) : m_ToCall(toCall) {}
    ~OnScopeExit() { if(m_ToCall) m_ToCall(); }
  };
}

#define MP_ConcatHelper(L, R) L ## R
#define MP_Concat(L, R) MP_ConcatHelper(L, R)
#define MP_OnScopeExit mpInternal::OnScopeExit<std::function<void()>> MP_Concat(_scopeExit_, __LINE__) = [&]

#define MP_Min(lhs, rhs) ((lhs) < (rhs) ? (lhs) : (rhs))

#include "mpWrapper/Result.h"
#include "mpWrapper/Assert.h"
#include "mpWrapper/Logging.h"
#include "mpWrapper/Error.h"

#define MP_NotImplemented MP_ReportError("Not implemented.")

#define Deref(x) (*(x))
