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

template<typename Type, typename IteratorType>
Type mpParse(IteratorType Begin, IteratorType End)
{
  return mpInternal::ParseHelper<Type>::Parse(Begin, End);
}
