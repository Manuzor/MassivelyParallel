#pragma once

namespace mpInternal
{
  bool MP_WrapperAPI ReportError(const char* szFile, size_t uiLine, const char* szMessage);
  bool MP_WrapperAPI AssertionFailed(const char* szFile, size_t uiLine, const char* szCondition, const char* szMessage);
  bool MP_WrapperAPI CLCallFailed(const char* szFile, size_t uiLine, const char* szCall, mpResult result);
}

MP_WrapperAPI const char* mpTranslateStatusCode(cl_int iStatusCode);

#define MP_DebugBreak do { __debugbreak(); } while(false)
#define MP_ReportError(...) if (mpInternal::ReportError(__FILE__, __LINE__, __VA_ARGS__)) MP_DebugBreak;
#define MP_Assert(condition, ...) do { if (!(condition)) { if (mpInternal::AssertionFailed(__FILE__, __LINE__, #condition, __VA_ARGS__)) MP_DebugBreak; } } while(false)

#define MP_Verify(clCall) do { mpResult result = (clCall); if(result.Failed()) { if (mpInternal::CLCallFailed(__FILE__, __LINE__, #clCall, result)) MP_DebugBreak; } } while(false)

#define MP_NotImplemented MP_ReportError("Not implemented.")
