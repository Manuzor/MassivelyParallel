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

template<typename ElementType, typename BufferType, typename IteratorType, size_t Stride = sizeof(ElementType)>
static size_t ReadRow(BufferType& BufferCurrent, BufferType BufferEnd, IteratorType& Current, IteratorType End)
{
  size_t uiElementsRead = 0;
  SkipWhiteSpace(Current, End);

  MP_Assert(*Current == '{', "Invalid syntax");
  ++Current; // Read the leading '}' character.

  auto BufferStart = BufferCurrent;
  std::string sTemp;

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

    MP_Assert(BufferCurrent < BufferEnd, "Buffer not big enough!");

    sTemp.assign(ValueStart, ValueEnd);
    auto TheValue = mpParse<ElementType>(ValueStart, ValueEnd);
    memcpy(BufferCurrent, &TheValue, sizeof(TheValue));
    BufferCurrent += Stride;
  }

  MP_Assert(*Current == '}', "Invalid syntax");
  ++Current; // Read trailing '}' character of the row

  return (BufferCurrent - BufferStart) / sizeof(ElementType);
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
  auto BufferCurrent = reinterpret_cast<char*>(out_Data);
  auto BufferEnd = BufferCurrent + (uiWidth * uiHeight) * sizeof(cl_float);

  while(true)
  {
    SkipWhiteSpace(Current, End);
    if (Current == End)
      break;

    ReadRow<cl_float>(BufferCurrent, BufferEnd, Current, End);

    SkipWhiteSpace(Current, End);

    if (*Current == '}')
      break;

    MP_Assert(*Current == ',', "Invalid syntax");
    ++Current;
  }
}
