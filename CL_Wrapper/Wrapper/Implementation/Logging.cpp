#include "Wrapper/PCH.h"
#include "Wrapper/Logging.h"
#include "Wrapper/String.h"

static void LogToStdOut(const std::string& sMessage)
{
  printf("%s\n", sMessage.c_str());
}

static void LogToVisualStudio(const std::string& sMessage)
{
  OutputDebugStringA(sMessage.c_str());
  OutputDebugStringA("\n");
}

#define BODY \
  std::ostringstream outMessage; \
  va_list vargs;\
  va_start(vargs, szFormat);\
  MP_OnScopeExit { va_end(vargs); };\
  mpString::AppendFormatV(outMessage, szFormat, vargs);\
  auto sMessage = outMessage.str();\
  LogToStdOut(sMessage);\
  LogToVisualStudio(sMessage);

void MP_WrapperAPI mpLog::Info(const char* szFormat, ...)
{
  BODY
}

void MP_WrapperAPI mpLog::Success(const char* szFormat, ...)
{
  BODY
}

void MP_WrapperAPI mpLog::Warning(const char* szFormat, ...)
{
  BODY
}

void MP_WrapperAPI mpLog::Error(const char* szFormat, ...)
{
  BODY
}

#undef BODY
