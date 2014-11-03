#include "Wrapper/PCH.h"

void mpInternal::ReportError(const char* szFile, size_t uiLine, const char* szMessage)
{
  mpLog::Error("%s(%u): Error: %s", szFile, uiLine, szMessage);
  MP_DebugBreak;
}

void mpInternal::AssertionFailed(const char* szFile, size_t uiLine, const char* szCondition, const char* szMessage)
{
  mpLog::Error("%s(%u): Assertion failed.\n  Condition: %s\n  Message: %s", szFile, uiLine, szCondition, szMessage);
  MP_DebugBreak;
}

void mpInternal::CLCallFailed(const char* szFile, size_t uiLine, const char* szCall, mpResult result)
{
  mpLog::Error("%s(%u): CL call failed with status code \"%s\"\n  Call: %s", szFile, uiLine, mpTranslateStatusCode(result.m_Status), szCall);
  MP_DebugBreak;
}
