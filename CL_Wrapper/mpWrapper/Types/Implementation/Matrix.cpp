#include "mpWrapper/PCH.h"
#include "mpWrapper/Types/Matrix.h"
#include "mpWrapper/Utilities/File.h"
#include "mpWrapper/Utilities/String.h"

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

template<typename ElementType, typename BufferType, typename IteratorType>
static size_t ReadRow(BufferType& out_Buffer, IteratorType& Current, IteratorType End)
{
  size_t uiNumElementsAtBeginning = out_Buffer.size();
  SkipWhiteSpace(Current, End);

  MP_Assert(*Current == '{', "Invalid syntax");
  ++Current; // Read the leading '}' character.

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

    sTemp.assign(ValueStart, ValueEnd);
    auto TheValue = mpString::mpParse<ElementType>(ValueStart, ValueEnd);
    out_Buffer.push_back(TheValue);
  }

  MP_Assert(*Current == '}', "Invalid syntax");
  ++Current; // Read trailing '}' character of the row

  return out_Buffer.size() - uiNumElementsAtBeginning;
}

void mpInternal::mpLoadMatrixFromFile(std::vector<cl_float>& out_Data, size_t& out_uiHeight, size_t& out_uiWidth, const char* szFileName)
{
  out_uiHeight = 0;
  out_uiWidth = -1;

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

    auto ReadWidth = ReadRow<cl_float>(out_Data, Current, End);
    if (out_uiWidth == -1)
      out_uiWidth = ReadWidth;

    MP_Assert(ReadWidth == out_uiWidth, "Rows in matrix file do not all have the same width");

    ++out_uiHeight;

    SkipWhiteSpace(Current, End);

    if (*Current == '}')
      break;

    MP_Assert(*Current == ',', "Invalid syntax");
    ++Current;
  }
}
