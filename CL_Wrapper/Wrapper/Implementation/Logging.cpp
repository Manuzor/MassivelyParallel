#include "Wrapper/PCH.h"
#include "Wrapper/Logging.h"
#include "Wrapper/String.h"

static mpInternal::LogBlock* g_pCurrentBlock;

mpInternal::LogBlock::LogBlock(const char* szFormat, ...) :
  m_pParent(g_pCurrentBlock),
  m_bHasLogged(false)
{
  g_pCurrentBlock = this;
  m_uiIndentation = m_pParent ? m_pParent->m_uiIndentation + 1 : 1;
}

mpInternal::LogBlock::~LogBlock()
{
  g_pCurrentBlock = m_pParent;
  m_pParent = nullptr;
}

void mpInternal::LogBlock::LogDescription()
{
  if(m_bHasLogged)
    return;

}

namespace
{

  const char* GetString(mpInternal::LogLevel Level)
  {
    switch(Level)
    {
    case mpInternal::LogLevel::BlockBegin: return "BlockBegin";
    case mpInternal::LogLevel::BlockEnd:   return "BlockEnd";
    case mpInternal::LogLevel::Info:       return "Info";
    case mpInternal::LogLevel::Success:    return "Success";
    case mpInternal::LogLevel::Warning:    return "Warning";
    case mpInternal::LogLevel::Error:      return "Error";
    }
    MP_NotImplemented;
    return nullptr;
  }

  const char* GetShortString(mpInternal::LogLevel Level)
  {
    switch(Level)
    {
    case mpInternal::LogLevel::BlockBegin: return ">>>";
    case mpInternal::LogLevel::BlockEnd:   return "<<<";
    case mpInternal::LogLevel::Info:       return "Ifo";
    case mpInternal::LogLevel::Success:    return "Suc";
    case mpInternal::LogLevel::Warning:    return "Wrn";
    case mpInternal::LogLevel::Error:      return "Err";
    }
    MP_NotImplemented;
    return nullptr;
  }
}

static void LogToStdOut(mpInternal::LogLevel Level, const std::string& sMessage)
{
  printf("%s| %s\n", GetShortString(Level), sMessage.c_str());
}

static void LogToVisualStudio(mpInternal::LogLevel Level, const std::string& sMessage)
{
  OutputDebugStringA(GetShortString(Level));
  OutputDebugStringA("| ");
  OutputDebugStringA(sMessage.c_str());
  OutputDebugStringA("\n");
}

#define FORMAT_MESSAGE(outMessage, szFormat) \
  do{\
    va_list vargs;\
    va_start(vargs, szFormat);\
    MP_OnScopeExit { va_end(vargs); };\
    mpString::AppendFormatV(outMessage, szFormat, vargs);\
  } while(false)

void MP_WrapperAPI mpLog::Info(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(mpInternal::LogLevel::Info, ssMessage.str());
}

void MP_WrapperAPI mpLog::Success(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(mpInternal::LogLevel::Success, ssMessage);
}

void MP_WrapperAPI mpLog::Warning(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(mpInternal::LogLevel::Warning, ssMessage);
}

void MP_WrapperAPI mpLog::Error(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(mpInternal::LogLevel::Error, ssMessage);
}

#undef FORMAT_MESSAGE

void mpInternal::DoLog(LogLevel Level, std::stringstream& ssMessage)
{
  if(g_pCurrentBlock)
  {
    for(mpUInt64 i = 0; i < g_pCurrentBlock->GetIndentation(); ++i)
    {
      ssMessage << "  ";
    }
  }
  auto sMessage = ssMessage.str();
  LogToStdOut(Level, sMessage);
  LogToVisualStudio(Level, sMessage);
}
