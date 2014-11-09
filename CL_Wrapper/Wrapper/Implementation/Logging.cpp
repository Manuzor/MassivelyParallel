#include "Wrapper/PCH.h"
#include "Wrapper/Logging.h"
#include "Wrapper/String.h"

namespace
{
  enum class LogLevel
  {
    Info,
    Success,
    Warning,
    Error
  };

  const char* GetString(LogLevel Level)
  {
    switch(Level)
    {
    case LogLevel::Info:    return "Info";
    case LogLevel::Success: return "Success";
    case LogLevel::Warning: return "Warning";
    case LogLevel::Error:   return "Error";
    }
    MP_NotImplemented;
    return nullptr;
  }

  const char* GetShortString(LogLevel Level)
  {
    switch(Level)
    {
    case LogLevel::Info:    return "Ifo";
    case LogLevel::Success: return "Suc";
    case LogLevel::Warning: return "Wrn";
    case LogLevel::Error:   return "Err";
    }
    MP_NotImplemented;
    return nullptr;
  }
}

static void LogToStdOut(LogLevel Level, const std::string& sMessage)
{
  printf("%s| %s\n", GetShortString(Level), sMessage.c_str());
}

static void LogToVisualStudio(LogLevel Level, const std::string& sMessage)
{
  OutputDebugStringA(GetShortString(Level));
  OutputDebugStringA("| ");
  OutputDebugStringA(sMessage.c_str());
  OutputDebugStringA("\n");
}

#define BODY(Level) \
  std::ostringstream outMessage; \
  va_list vargs;\
  va_start(vargs, szFormat);\
  MP_OnScopeExit { va_end(vargs); };\
  mpString::AppendFormatV(outMessage, szFormat, vargs);\
  auto sMessage = outMessage.str();\
  LogToStdOut(Level, sMessage);\
  LogToVisualStudio(Level, sMessage);

void MP_WrapperAPI mpLog::Info(const char* szFormat, ...)
{
  BODY(LogLevel::Info);
}

void MP_WrapperAPI mpLog::Success(const char* szFormat, ...)
{
  BODY(LogLevel::Success);
}

void MP_WrapperAPI mpLog::Warning(const char* szFormat, ...)
{
  BODY(LogLevel::Warning);
}

void MP_WrapperAPI mpLog::Error(const char* szFormat, ...)
{
  BODY(LogLevel::Error);
}

#undef BODY
