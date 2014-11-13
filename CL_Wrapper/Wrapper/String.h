#pragma once

namespace mpString
{
  MP_WrapperAPI void AppendFormat(std::stringstream& out_Formatted, const char* szFormat, ...);
  MP_WrapperAPI void AppendFormatV(std::stringstream& out_Formatted, const char* szFormat, va_list args);
}
