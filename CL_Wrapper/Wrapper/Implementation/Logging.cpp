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
    /// \brief Special constructor for the root-block
    LogBlock() :
      m_pParent(nullptr),
      m_uiIndentation(0),
      m_bNeverDoAnything(true)
    {
    }

    LogBlock(LogBlock* pParent, std::string& sMessage) :
      m_pParent(pParent),
      m_uiIndentation(m_pParent->m_uiIndentation + 1),
      m_sMessage(std::move(sMessage))
    {
      MP_Assert(m_pParent, "Invalid parent! Are you using log blocks after shutdown?");
    }

    ~LogBlock()
    {
      if(m_bNeverDoAnything || !m_bHasLogged)
        return;

      PropagateLogMessage(LogLevel::BlockEnd, m_sMessage);
    }

    void LogMessage()
    {
      if(m_bNeverDoAnything || m_bHasLogged)
        return;

      m_bHasLogged = true;
      PropagateLogMessage(LogLevel::BlockBegin, m_sMessage);
    }

    MP_ForceInline size_t GetIndentation() const { return m_uiIndentation; }
    MP_ForceInline LogBlock* GetParent() { return m_pParent; }

  private:
    LogBlock* m_pParent;
    size_t m_uiIndentation = 1;
    std::string m_sMessage;
    bool m_bHasLogged = false;
    bool m_bNeverDoAnything = false;
  };

  static LogBlock* g_pCurrentBlock;
}

MP_GlobalInitializationBegin

  LogBlock m_RootLogBlock;

  MP_OnGlobalStartup
  {
    g_pCurrentBlock = &m_RootLogBlock;
  }

  MP_OnGlobalShutdown
  {
    MP_Assert(g_pCurrentBlock == &m_RootLogBlock, "No log blocks may be open on shutdown!");
    g_pCurrentBlock = nullptr;
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
  g_pCurrentBlock->LogMessage();

  auto sMessage = ssFormattedMessage.str();
  PropagateLogMessage(Level, ssFormattedMessage.str());
}

static void LogToStdOut(LogLevel Level, const std::string& sMessage)
{
  printf("%s| ", GetShortString(Level));

  auto uiIndentation = g_pCurrentBlock->GetIndentation();
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

  auto uiIndentation = g_pCurrentBlock->GetIndentation();
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
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);

  g_pCurrentBlock = new LogBlock(g_pCurrentBlock, ssMessage.str());
}

void mpLog::BlockEnd()
{
  MP_Assert(g_pCurrentBlock, "Called BlockEnd more often than BlockBegin!");

  auto pParentBlock = g_pCurrentBlock->GetParent();
  MP_Assert(pParentBlock, "Called BlockEnd more often than BlockBegin!");

  delete g_pCurrentBlock;
  g_pCurrentBlock = pParentBlock;
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
