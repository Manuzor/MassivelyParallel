#pragma once

namespace mpString
{
  MP_WrapperAPI void AppendFormat(std::ostringstream& out_Formatted, const char* szFormat, ...);
  MP_WrapperAPI void AppendFormatV(std::ostringstream& out_Formatted, const char* szFormat, va_list args);
}
