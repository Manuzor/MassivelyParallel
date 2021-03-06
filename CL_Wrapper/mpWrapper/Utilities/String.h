#pragma once

namespace mpInternal
{
  template<typename Type>
  struct ParseHelper;

  template<>
  struct ParseHelper<cl_float>
  {
    template<typename IteratorType>
    static cl_float Parse(IteratorType Begin, IteratorType End)
    {
      std::string s(Begin, End);
      return std::stof(s);
    }
  };

  template<>
  struct ParseHelper<cl_int>
  {
    template<typename IteratorType>
    static cl_int Parse(IteratorType Begin, IteratorType End)
    {
      std::string s(Begin, End);
      return std::stoi(s);
    }
  };
}

namespace mpString
{
  MP_WrapperAPI void mpAppendFormat(std::stringstream& out_Formatted, const char* szFormat, ...);
  MP_WrapperAPI void mpAppendFormatV(std::stringstream& out_Formatted, const char* szFormat, va_list args);

  template<typename Type, typename IteratorType>
  Type mpParse(IteratorType Begin, IteratorType End)
  {
    return mpInternal::ParseHelper<Type>::Parse(Begin, End);
  }

  template<typename CharType>
  void mpCreateCopy(const CharType*& out_szDestination, const CharType* szSource, size_t uiCount)
  {
    MP_Assert(out_szDestination == nullptr, "This function allocates new memory for out_szDestination.");
    auto szBuffer = new CharType[uiCount + 1];
    szBuffer[uiCount] = '\0';
    memcpy(szBuffer, szSource, uiCount);
    out_szDestination = szBuffer;
  }


  template<typename CharType>
  void mpDestroyCopy(const CharType*& szString)
  {
    delete[] szString;
    szString = nullptr;
  }
}
