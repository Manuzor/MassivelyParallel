#include "mpWrapper/PCH.h"
#include "mpWrapper/Utilities/String.h"
#include "mpWrapper/Foundation/Startup.h"

static mpLogLevel g_CurrentLogLevel = mpLogLevel::All;

void PropagateLogMessage(mpLogLevel Level, const std::string& sMessage, size_t uiIndentation);

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

      if(m_pParent)
        m_pParent->LogMessage();

      PropagateLogMessage(mpLogLevel::BlockEnd, m_sMessage, GetLocalIndentation());
      m_bNeverDoAnything = true;
    }

    void LogMessage()
    {
      if(m_bNeverDoAnything || m_bHasLogged)
        return;

      if(m_pParent)
        m_pParent->LogMessage();

      PropagateLogMessage(mpLogLevel::BlockBegin, m_sMessage, GetLocalIndentation());
      m_bHasLogged = true;
    }

    MP_ForceInline size_t GetIndentation() const { return m_uiIndentation; }
    MP_ForceInline LogBlock* GetParent() { return m_pParent; }

  private:
    LogBlock* m_pParent;
    size_t m_uiIndentation = 1;
    std::string m_sMessage;
    bool m_bHasLogged = false;
    bool m_bNeverDoAnything = false;

    MP_ForceInline size_t GetLocalIndentation() const { return m_uiIndentation > 0 ? m_uiIndentation - 1 : 0; }
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

static void DoLog(mpLogLevel Level, std::stringstream& ssFormattedMessage)
{
  g_pCurrentBlock->LogMessage();

  auto sMessage = ssFormattedMessage.str();
  PropagateLogMessage(Level, ssFormattedMessage.str(), g_pCurrentBlock->GetIndentation());
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

static void LogToStdOut(mpLogLevel Level, const std::string& sMessage, size_t uiIndentation)
{
  CONSOLE_SCREEN_BUFFER_INFO PreviousConsoleState;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &PreviousConsoleState);
  MP_OnScopeExit{ SetConsoleColor(PreviousConsoleState.wAttributes); };

  auto szPrefix = "";

  static_assert(mpLogLevel::COUNT == 8, "Update this function.");
  switch(Level)
  {
  case mpLogLevel::BlockBegin:
    SetConsoleColor(ConsoleColor::DarkGreen);
    szPrefix = ">> ";
    break;
  case mpLogLevel::BlockEnd:
    SetConsoleColor(ConsoleColor::DarkGreen);
    szPrefix = "<< ";
    break;
  case mpLogLevel::Error:
    SetConsoleColor(ConsoleColor::BrightRed);
    break;
  case mpLogLevel::SeriousWarning:
    SetConsoleColor(ConsoleColor::BrightMagenta);
    break;
  case mpLogLevel::Warning:
    SetConsoleColor(ConsoleColor::BrightYellow);
    break;
  case mpLogLevel::Success:
    SetConsoleColor(ConsoleColor::BrightGreen);
    break;
  case mpLogLevel::Info:
    SetConsoleColor(ConsoleColor::Default);
    break;
  case mpLogLevel::Dev:
    SetConsoleColor(ConsoleColor::Gray);
    break;
  case mpLogLevel::Debug:
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

static void LogToVisualStudio(mpLogLevel Level, const std::string& sMessage, size_t uiIndentation)
{
  // Prefix
  OutputDebugStringA(Level.GetShortString());
  OutputDebugStringA("| ");

  auto szPrefix = "";

  static_assert(mpLogLevel::COUNT == 8, "Update this function.");
  switch(Level)
  {
  case mpLogLevel::BlockBegin:
    szPrefix = ">> ";
    break;
  case mpLogLevel::BlockEnd:
    szPrefix = "<< ";
    break;
  case mpLogLevel::Error:
  case mpLogLevel::SeriousWarning:
  case mpLogLevel::Warning:
  case mpLogLevel::Success:
  case mpLogLevel::Info:
  case mpLogLevel::Dev:
  case mpLogLevel::Debug:
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

static void PropagateLogMessage(mpLogLevel Level, const std::string& sMessage, size_t uiIndentation)
{
  LogToStdOut      (Level, sMessage, uiIndentation);
  LogToVisualStudio(Level, sMessage, uiIndentation);
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
  auto Level = mpLogLevel::Error;
  if(g_CurrentLogLevel < Level)
    return;

  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(Level, ssMessage);
}

void mpLog::SeriousWarning(const char* szFormat, ...)
{
  auto Level = mpLogLevel::SeriousWarning;
  if(g_CurrentLogLevel < Level)
    return;

  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(Level, ssMessage);
}

void mpLog::Warning(const char* szFormat, ...)
{
  auto Level = mpLogLevel::Warning;
  if(g_CurrentLogLevel < Level)
    return;

  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(Level, ssMessage);
}

void mpLog::Success(const char* szFormat, ...)
{
  auto Level = mpLogLevel::Success;
  if(g_CurrentLogLevel < Level)
    return;

  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(Level, ssMessage);
}

void mpLog::Info(const char* szFormat, ...)
{
  auto Level = mpLogLevel::Info;
  if(g_CurrentLogLevel < Level)
    return;

  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(Level, ssMessage);
}

void mpLog::Dev(const char* szFormat, ...)
{
  auto Level = mpLogLevel::Dev;
  if(g_CurrentLogLevel < Level)
    return;

  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(Level, ssMessage);
}

void mpLog::Debug(const char* szFormat, ...)
{
  auto Level = mpLogLevel::Debug;
  if(g_CurrentLogLevel < Level)
    return;

  std::stringstream ssMessage;
  FORMAT_MESSAGE(ssMessage, szFormat);
  DoLog(Level, ssMessage);
}

void mpLog::SetLogLevel(mpLogLevel Level)
{
  g_CurrentLogLevel = Level;
}

mpLogLevel mpLog::GetLogLevel()
{
  return g_CurrentLogLevel;
}

#undef FORMAT_MESSAGE
