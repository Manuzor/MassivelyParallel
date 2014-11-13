#pragma once

namespace mpInternal
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

  struct MP_WrapperAPI LogBlock
  {
    LogBlock(const char* szFormat, ...);
    ~LogBlock();

    void LogDescription();
    mpUInt64 GetIndentation() const { return m_uiIndentation; }

  private:
    LogBlock* m_pParent;
    std::string m_Description;
    mpUInt64 m_uiIndentation;
    bool m_bHasLogged;
  };

  void MP_WrapperAPI DoLog(LogLevel Level, std::stringstream& ssMessage);
}

namespace mpLog
{
  void MP_WrapperAPI Info(const char* szFormat, ...);
  void MP_WrapperAPI Success(const char* szFormat, ...);
  void MP_WrapperAPI Warning(const char* szFormat, ...);
  void MP_WrapperAPI Error(const char* szFormat, ...);
}

#define MP_LogBlock ::mpInternal::LogBlock MP_Concat(_LogBlock_, __LINE__)
