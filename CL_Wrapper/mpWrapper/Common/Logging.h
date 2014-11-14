#pragma once

namespace mpLog
{
  /// \note Don't forget to call BlockEnd! Or use MP_LogBlock
  MP_WrapperAPI void BlockBegin(const char* szFormat, ...);
  MP_WrapperAPI void BlockEnd();

  MP_WrapperAPI void Info(const char* szFormat, ...);
  MP_WrapperAPI void Success(const char* szFormat, ...);
  MP_WrapperAPI void Warning(const char* szFormat, ...);
  MP_WrapperAPI void Error(const char* szFormat, ...);
}

#define MP_LogBlock(...) ::mpLog::BlockBegin(__VA_ARGS__);\
        MP_OnScopeExit { ::mpLog::BlockEnd(); }
