#include "mpWrapper/PCH.h"

void mpInternal::ReportError(const char* szFile, size_t uiLine, const char* szMessage)
{
  mpLog::Error("=== Error Report ===\n%s(%u): %s", szFile, uiLine, szMessage);
  MP_DebugBreak;
}

void mpInternal::AssertionFailed(const char* szFile, size_t uiLine, const char* szCondition, const char* szMessage)
{
  mpLog::Error("=== Assertion Failed ===\n%s(%u): %s\n  Message: %s", szFile, uiLine, szCondition, szMessage);
  MP_DebugBreak;
}

void mpInternal::CLCallFailed(const char* szFile, size_t uiLine, const char* szCall, mpResult result)
{
  mpLog::Error("CL call failed with status code \"%s\":\n%s(%u): %s", mpTranslateStatusCode(result.m_Status), szFile, uiLine, szCall);
  MP_DebugBreak;
}
