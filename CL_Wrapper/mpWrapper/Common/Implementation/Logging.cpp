#include "mpWrapper/PCH.h"
#include "mpWrapper/Utilities/String.h"
#include "mpWrapper/Foundation/Startup.h"

namespace
{
  struct LogLevel
  {
    enum Enum
    {
      BlockBegin = -2,
      BlockEnd = -1,

      Error,
      SeriousWarning,
      Warning,
      Success,
      Info,
      Dev,
      Debug,

      COUNT
    };

    Enum m_Value;
    LogLevel(Enum Value) : m_Value(Value) {}

    MP_ForceInline operator Enum() const { return m_Value; }

    const char* GetString()
    {
      static_assert(COUNT == 7, "Update this function.");
      switch(m_Value)
      {
      case LogLevel::BlockBegin:     return "BlockBegin";
      case LogLevel::BlockEnd:       return "BlockEnd";
      case LogLevel::Error:          return "Error";
      case LogLevel::SeriousWarning: return "SeriousWarning";
      case LogLevel::Warning:        return "Warning";
      case LogLevel::Success:        return "Success";
      case LogLevel::Info:           return "Info";
      case LogLevel::Dev:            return "Dev";
      case LogLevel::Debug:          return "Debug";
      }
      MP_NotImplemented;
      return nullptr;
    }

    const char* GetShortString()
    {
      static_assert(COUNT == 7, "Update this function.");
      switch(m_Value)
      {
      case LogLevel::BlockBegin:     return ">>>";
      case LogLevel::BlockEnd:       return "<<<";
      case LogLevel::Error:          return "Err";
      case LogLevel::SeriousWarning: return "Srs";
      case LogLevel::Warning:        return "Wen";
      case LogLevel::Success:        return "Suc";
      case LogLevel::Info:           return "Ifo";
      case LogLevel::Dev:            return "Dev";
      case LogLevel::Debug:          return "Dbg";
      }
      MP_NotImplemented;
      return nullptr;
    }
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

MP_GlobalInitializationEnd

static void DoLog(LogLevel Level, std::stringstream& ssFormattedMessage)
{
  g_pCurrentBlock->LogMessage();

  auto sMessage = ssFormattedMessage.str();
  PropagateLogMessage(Level, ssFormattedMessage.str());
}
namespace
{
  struct ConsoleColor
  {
    enum Enum
    {
      Black = 0x00,

      DarkBlue      = 0x01,
      DarkGreen     = 0x02,
      DarkCyan      = 0x03,
      DarkRed       = 0x04,
      DarkMagenta   = 0x05,
      DarkYellow    = 0x06,

      Silver        = 0x07,
      Gray          = 0x08,

      BrightBlue    = 0x09,
      BrightGreen   = 0x0A,
      BrightCyan    = 0x0B,
      BrightRed     = 0x0C,
      BrightMagenta = 0x0D,
      BrightYellow  = 0x0E,

      White         = 0x0F,

      Default = Silver
    };
  };
}

static void SetConsoleColor(WORD iColor)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), iColor);
}

static void LogToStdOut(LogLevel Level, const std::string& sMessage)
{
  CONSOLE_SCREEN_BUFFER_INFO PreviousConsoleState;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &PreviousConsoleState);
  MP_OnScopeExit{ SetConsoleColor(PreviousConsoleState.wAttributes); };

  auto uiIndentation = g_pCurrentBlock->GetIndentation();
  auto szPrefix = "";

  static_assert(LogLevel::COUNT == 7, "Update this function.");
  switch(Level)
  {
  case LogLevel::BlockBegin:
    SetConsoleColor(ConsoleColor::DarkGreen);
    szPrefix = ">> ";
    --uiIndentation;
    break;
  case LogLevel::BlockEnd:
    SetConsoleColor(ConsoleColor::DarkGreen);
    szPrefix = "<< ";
    --uiIndentation;
    break;
  case LogLevel::Error:
    SetConsoleColor(ConsoleColor::BrightRed);
    break;
  case LogLevel::SeriousWarning:
    SetConsoleColor(ConsoleColor::BrightMagenta);
    break;
  case LogLevel::Warning:
    SetConsoleColor(ConsoleColor::BrightYellow);
    break;
  case LogLevel::Success:
    SetConsoleColor(ConsoleColor::BrightGreen);
    break;
  case LogLevel::Info:
    SetConsoleColor(ConsoleColor::Default);
    break;
  case LogLevel::Dev:
    SetConsoleColor(ConsoleColor::Gray);
    break;
  case LogLevel::Debug:
    SetConsoleColor(ConsoleColor::BrightCyan);
    break;
  default:
    MP_NotImplemented;
    break;
  }

  printf("%s| ", Level.GetShortString());
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
  OutputDebugStringA(Level.GetShortString());
  OutputDebugStringA("| ");

  auto uiIndentation = g_pCurrentBlock->GetIndentation();
  auto szPrefix = "";

  static_assert(LogLevel::COUNT == 7, "Update this function.");
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
  case LogLevel::Error:
  case LogLevel::SeriousWarning:
  case LogLevel::Warning:
  case LogLevel::Success:
  case LogLevel::Info:
  case LogLevel::Dev:
  case LogLevel::Debug:
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
    mpString::mpAppendFormatV(outMessage, szFormat, vargs);\
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

void mpLog::Error(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Error, ssMessage);
}

void mpLog::SeriousWarning(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::SeriousWarning, ssMessage);
}

void mpLog::Warning(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Warning, ssMessage);
}

void mpLog::Success(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Success, ssMessage);
}

void mpLog::Info(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Info, ssMessage);
}

void mpLog::Dev(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Dev, ssMessage);
}

void mpLog::Debug(const char* szFormat, ...)
{
  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(LogLevel::Debug, ssMessage);
}

#undef FORMAT_MESSAGE
