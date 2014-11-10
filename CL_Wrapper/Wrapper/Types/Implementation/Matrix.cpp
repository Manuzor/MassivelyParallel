#include "Wrapper/PCH.h"
#include "Wrapper/Types/Matrix.h"
#include "Wrapper/Utilities/File.h"
#include "Wrapper/Utilities/String.h"

static bool IsWhiteSpace(char Character)
{
  return Character == ' '
      || Character == '\t'
      || Character == '\r'
      || Character == '\n'
      || Character == '\v'
      || Character == '\b';
}

template<typename IteratorType>
static size_t SkipWhiteSpace(IteratorType& Iterator, IteratorType End)
{
  auto Start = Iterator;
  while(Iterator != End && IsWhiteSpace(*Iterator))
    ++Iterator;

  return Iterator - Start;
}

template<typename ElementType, typename IteratorType>
static size_t ReadRow(size_t uiBufferSize, void* out_Buffer, IteratorType& Current, IteratorType End)
{
  size_t uiElementsRead = 0;
  SkipWhiteSpace(Current, End);

  MP_Assert(*Current == '{', "Invalid syntax");
  ++Current; // Read the leading '}' character.

  std::string sTemp;
  size_t uiIndex = 0;
  size_t uiNumParsedElements = 0;

  while(true)
  {
    SkipWhiteSpace(Current, End);

    if (Current == End || *Current == '}')
      break;

    auto ValueStart = Current;
    while(true)
    {
      if(*Current == ',' || *Current == '}')
        break;
      ++Current;
    }

    auto ValueEnd = Current;

    if (Current != End && *Current == ',') // Could also be '}'
      ++Current;

    MP_Assert(uiIndex < uiBufferSize, "Buffer not big enough!");

    sTemp.assign(ValueStart, ValueEnd);
    auto TheValue = mpParse<ElementType>(ValueStart, ValueEnd);
    memcpy(reinterpret_cast<char*>(out_Buffer) + uiIndex, &TheValue, sizeof(TheValue));
    uiIndex += sizeof(ElementType);
    ++uiNumParsedElements;
  }

  MP_Assert(*Current == '}', "Invalid syntax");
  ++Current; // Read trailing '}' character of the row

  return uiNumParsedElements;
}

void mpLoadMatrixFromFile(const char* szFileName,
                          size_t uiWidth, size_t uiHeight, void* out_Data)
{
  std::vector<char> Content;
  mpLoadStringFromFile(Content, szFileName);

  auto End = Content.cend();
  auto Current = Content.cbegin();

  SkipWhiteSpace(Current, End);

  if (*Current != '{')
  {
    MP_ReportError("Invalid matrix file syntax.");
  }

  ++Current;

  while(true)
  {
    SkipWhiteSpace(Current, End);
    if (Current == End)
      break;

    ReadRow<cl_float>(uiWidth * uiHeight, out_Data, Current, End);

    SkipWhiteSpace(Current, End);

    if (*Current == '}')
      break;

    MP_Assert(*Current == ',', "Invalid syntax");
    ++Current;
  }
}
