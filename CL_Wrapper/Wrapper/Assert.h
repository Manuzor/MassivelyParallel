#pragma once
#include "Wrapper/Result.h"

namespace mpInternal
{
  void MP_WrapperAPI ReportError(const char* szFile, size_t uiLine, const char* szMessage);
  void MP_WrapperAPI AssertionFailed(const char* szFile, size_t uiLine, const char* szCondition, const char* szMessage);
  void MP_WrapperAPI CLCallFailed(const char* szFile, size_t uiLine, const char* szCall, mpResult result);
}

#define MP_DebugBreak do { __debugbreak(); } while(false)
#define MP_ReportError(...) mpInternal::ReportError(__FILE__, __LINE__, __VA_ARGS__)
#define MP_Assert(condition, ...) do { if(!(condition)) { mpInternal::AssertionFailed(__FILE__, __LINE__, #condition, __VA_ARGS__); } } while(false)

#define MP_Verify(clCall) do { mpResult result = (clCall); if(result.Failed()) { mpInternal::CLCallFailed(__FILE__, __LINE__, #clCall, result); } } while(false)
