#pragma once

struct mpLogLevel
{
  enum Enum
  {
    BlockBegin = -2,
    BlockEnd = -1,

    None = 0,

    Error,
    SeriousWarning,
    Warning,
    Success,
    Info,
    Dev,
    Debug,

    All = Debug,

    COUNT
  };

  Enum m_Value;
  mpLogLevel(Enum Value) : m_Value(Value) {}

  MP_ForceInline operator Enum() const { return m_Value; }

  /// \brief Gets a string representation of this instances' m_Value.
  /// \example mpLogLevel::Info => "Info"
  const char* GetString();

  /// \brief Gets a 3-character string representation of this instances' m_Value.
  /// \example mpLogLevel::Info => "Ifo" or mpLogLevel:: BlockBegin => ">>>"
  const char* GetShortString();
};

namespace mpLog
{
  /// \note Don't forget to call BlockEnd! Or use MP_LogBlock
  MP_WrapperAPI void BlockBegin(const char* szFormat, ...);
  MP_WrapperAPI void BlockEnd();

  MP_WrapperAPI void Error(const char* szFormat, ...);
  MP_WrapperAPI void SeriousWarning(const char* szFormat, ...);
  MP_WrapperAPI void Warning(const char* szFormat, ...);
  MP_WrapperAPI void Success(const char* szFormat, ...);
  MP_WrapperAPI void Info(const char* szFormat, ...);
  MP_WrapperAPI void Dev(const char* szFormat, ...);
  MP_WrapperAPI void Debug(const char* szFormat, ...);

  MP_WrapperAPI void SetLogLevel(mpLogLevel Level);
  MP_WrapperAPI mpLogLevel GetLogLevel();
}

#include "mpWrapper/Common/Implementation/Logging.inl"

#define MP_LogBlock(...) ::mpLog::BlockBegin(__VA_ARGS__);\
        MP_OnScopeExit { ::mpLog::BlockEnd(); }
