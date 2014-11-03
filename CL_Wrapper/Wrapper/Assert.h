#pragma once
#include "Wrapper/Result.h"

namespace mpInternal
{
  void DebugBreak();

  void ReportError(const char* szFile, size_t uiLine, const char* szMessage);
  void AssertionFailed(const char* szFile, size_t uiLine, const char* szCondition, const char* szMessage);
  void CLCallFailed(const char* szFile, size_t uiLine, const char* szCall, mpResult result);
}

#define MP_ReportError(...) mpInternal::ReportError(__FILE__, __LINE__, __VA_ARGS__)
#define MP_Assert(condition, ...) do { if(!(condition)) { mpInternal::AssertionFailed(__FILE__, __LINE__, #condition, __VA_ARGS__); } } while(false)

#define MP_Verify(clCall) do { mpResult result = (clCall); if(result.Failed()) { mpInternal::CLCallFailed(__FILE__, __LINE__, #clCall, result); } } while(false)
