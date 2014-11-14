#include "Wrapper/PCH.h"
#include "Wrapper/Logging.h"
#include "Wrapper/String.h"
#include "Wrapper/Startup.h"

namespace
{
  enum class LogLevel
  {
    BlockBegin = -2,
    BlockEnd = -1,

    Info,
    Success,
    Warning,
    Error
  };
}

void PropagateLogMessage(LogLevel Level, const std::string& sMessage);

namespace
{
  struct LogBlock
  {
    LogBlock() {}

    LogBlock(std::string& sMessage) :
      m_sMessage(std::move(sMessage)),
      m_bHasLogged(false)
    {
    }

    ~LogBlock()
    {
      if(!m_bHasLogged)
        return;

      PropagateLogMessage(LogLevel::BlockEnd, m_sMessage);
    }

    void LogMessage()
    {
      if(m_bHasLogged)
        return;

      m_bHasLogged = true;
      PropagateLogMessage(LogLevel::BlockBegin, m_sMessage);
    }

  private:
    std::string m_sMessage;
    bool m_bHasLogged;
  };

  static const size_t g_uiMaxLogBlocksCount = 64;
  static char g_LogBlocksmemory[g_uiMaxLogBlocksCount * sizeof(LogBlock)];
  static size_t g_uiLogBlocksCount = 0;

  MP_ForceInline static LogBlock* GetCurrentLogBlock()
  {
    if (g_uiLogBlocksCount == 0)
      return nullptr;

    auto uiIndex = (g_uiLogBlocksCount - 1) * sizeof(LogBlock);
    return reinterpret_cast<LogBlock*>(&g_LogBlocksmemory[uiIndex]);
  }
}

MP_GlobalInitializationBegin

  MP_OnGlobalShutdown
  {
    MP_Assert(g_uiLogBlocksCount == 0, "There must not be any open log blocks during shutdown!");
  }

MP_GlobalInitializationEnd

namespace
{
  const char* GetString(LogLevel Level)
  {
    switch(Level)
    {
    case LogLevel::BlockBegin: return "BlockBegin";
    case LogLevel::BlockEnd:   return "BlockEnd";
    case LogLevel::Info:       return "Info";
    case LogLevel::Success:    return "Success";
    case LogLevel::Warning:    return "Warning";
    case LogLevel::Error:      return "Error";
    }
    MP_NotImplemented;
    return nullptr;
  }

  const char* GetShortString(LogLevel Level)
  {
    switch(Level)
    {
    case LogLevel::BlockBegin: return ">>>";
    case LogLevel::BlockEnd:   return "<<<";
    case LogLevel::Info:       return "Ifo";
    case LogLevel::Success:    return "Suc";
    case LogLevel::Warning:    return "Wrn";
    case LogLevel::Error:      return "Err";
    }
    MP_NotImplemented;
    return nullptr;
  }
}

static void DoLog(LogLevel Level, std::stringstream& ssFormattedMessage)
{
  if(auto pBlock = GetCurrentLogBlock())
    pBlock->LogMessage();

  auto sMessage = ssFormattedMessage.str();
  PropagateLogMessage(Level, ssFormattedMessage.str());
}

static void LogToStdOut(LogLevel Level, const std::string& sMessage)
{
  printf("%s| ", GetShortString(Level));

  auto uiIndentation = g_uiLogBlocksCount;
  auto szPrefix = "";
  switch(Level)
  {
  case LogLevel::BlockBegin:
    szPrefix = ">> ";
    --uiIndentation;
    break;
  case LogLevel::BlockEnd:
    szPrefix = "<< ";
    --uiIndentation;
    break;
  case LogLevel::Info:
  case LogLevel::Success:
  case LogLevel::Warning:
  case LogLevel::Error:
    break;
  default:
    MP_NotImplemented;
    break;
  }
  auto szIndentationString = "  ";
  for(size_t i = 0; i < uiIndentation; ++i)
  {
    printf(szIndentationString);
  }
  printf("%s%s\n", szPrefix, sMessage.c_str());
}

static void LogToVisualStudio(LogLevel Level, const std::string& sMessage)
{
  // Prefix
  OutputDebugStringA(GetShortString(Level));
  OutputDebugStringA("| ");

  auto uiIndentation = g_uiLogBlocksCount;
  auto szPrefix = "";
  switch(Level)
  {
  case LogLevel::BlockBegin:
    szPrefix = ">> ";
    --uiIndentation;
    break;
  case LogLevel::BlockEnd:
    szPrefix = "<< ";
    --uiIndentation;
    break;
  case LogLevel::Info:
  case LogLevel::Success:
  case LogLevel::Warning:
  case LogLevel::Error:
    break;
  default:
    MP_NotImplemented;
    break;
  }

  auto szIndentationString = "  ";
  for(size_t i = 0; i < uiIndentation; ++i)
  {
    OutputDebugStringA(szIndentationString);
  }

  // The actual message
  OutputDebugStringA(szPrefix);
  OutputDebugStringA(sMessage.c_str());
  OutputDebugStringA("\n");
}

static void PropagateLogMessage(LogLevel Level, const std::string& sMessage)
{
  LogToStdOut(Level, sMessage);
  LogToVisualStudio(Level, sMessage);
}

#define FORMAT_MESSAGE(outMessage, szFormat) \
  do{\
    va_list vargs;\
    va_start(vargs, szFormat);\
    MP_OnScopeExit { va_end(vargs); };\
    mpString::AppendFormatV(outMessage, szFormat, vargs);\
  } while(false)

void mpLog::BlockBegin(const char* szFormat, ...)
{
  MP_Assert(g_uiLogBlocksCount < g_uiMaxLogBlocksCount,
            "Exceeded allowed number of nested log blocks.");

  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);

  new (g_LogBlocksmemory + (g_uiLogBlocksCount * sizeof(LogBlock))) LogBlock(ssMessage.str());
  ++g_uiLogBlocksCount;
}

void mpLog::BlockEnd()
{
  MP_Assert(g_uiLogBlocksCount > 0, "Called BlockEnd more often than BlockBegin!");
  GetCurrentLogBlock()->~LogBlock();
  --g_uiLogBlocksCount;
}

void MP_WrapperAPI mpLog::Info(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Info, ssMessage);
}

void MP_WrapperAPI mpLog::Success(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Success, ssMessage);
}

void MP_WrapperAPI mpLog::Warning(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Warning, ssMessage);
}

void MP_WrapperAPI mpLog::Error(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Error, ssMessage);
}

#undef FORMAT_MESSAGE
