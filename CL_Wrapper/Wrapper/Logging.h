#pragma once

namespace mpLog
{
  void MP_WrapperAPI Info(const char* szFormat, ...);
  void MP_WrapperAPI Success(const char* szFormat, ...);
  void MP_WrapperAPI Warning(const char* szFormat, ...);
  void MP_WrapperAPI Error(const char* szFormat, ...);
}
