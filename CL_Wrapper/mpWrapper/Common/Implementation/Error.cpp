#include "mpWrapper/PCH.h"

bool mpInternal::ReportError(const char* szFile, size_t uiLine, const char* szMessage)
{
  mpLog::Error("=== Error Report ===\n%s(%u): %s", szFile, uiLine, szMessage);
  return true;
}

bool mpInternal::AssertionFailed(const char* szFile, size_t uiLine, const char* szCondition, const char* szMessage)
{
  mpLog::Error("=== Assertion Failed ===\n%s(%u): %s\n  Message: %s", szFile, uiLine, szCondition, szMessage);
  return true;
}

bool mpInternal::CLCallFailed(const char* szFile, size_t uiLine, const char* szCall, mpResult result)
{
  mpLog::Error("CL call failed with status code \"%s\":\n%s(%u): %s", mpTranslateStatusCode(result.m_Status), szFile, uiLine, szCall);
  return true;
}
