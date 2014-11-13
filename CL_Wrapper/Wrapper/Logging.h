#pragma once

namespace mpLog
{
  struct MP_WrapperAPI Block
  {
    Block(const char* szFormat, ...);
    ~Block();

    void LogDescription();
    mpUInt64 GetIndentation() const { return m_uiIndentation; }

  private:
    Block* m_pParent;
    std::string m_Description;
    mpUInt64 m_uiIndentation;
    bool m_bHasLogged;
  };

  void MP_WrapperAPI Info(const char* szFormat, ...);
  void MP_WrapperAPI Success(const char* szFormat, ...);
  void MP_WrapperAPI Warning(const char* szFormat, ...);
  void MP_WrapperAPI Error(const char* szFormat, ...);
}

#define MP_LogBlock ::mpLog::Block MP_Concat(_LogBlock_, __LINE__)
