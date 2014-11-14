#include "Wrapper/PCH.h"
#include "Wrapper/String.h"

void mpString::AppendFormat(std::stringstream& out_Formatted, const char* szFormat, ...)
{
  va_list vargs;
  va_start(vargs, szFormat);
  MP_OnScopeExit { va_end(vargs); };
  AppendFormatV(out_Formatted, szFormat, vargs);
}

void mpString::AppendFormatV(std::stringstream& out_Formatted, const char* szFormat, va_list args)
{
  static const size_t maxAllocationSize(1024 * 1024 * 10); // 10 MB
  static const size_t bufferSize(1024);

  char buffer[bufferSize];

  int result = vsnprintf(buffer, bufferSize, szFormat, args);

  if(result < 0)
  {
    for(size_t allocationSize = 2 * bufferSize;
        allocationSize < maxAllocationSize;
        allocationSize *= 2)
    {
      char* largeBuffer = new char[allocationSize];
      MP_OnScopeExit{ delete[] largeBuffer; };

      result = vsnprintf(largeBuffer, allocationSize, szFormat, args);
      if(result >= 0)
      {
        out_Formatted << largeBuffer;
        return;
      }
    }
    MP_ReportError("The string you want to format is too large!");
  }

  out_Formatted << buffer;
}
