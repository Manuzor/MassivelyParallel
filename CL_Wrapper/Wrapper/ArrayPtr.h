#pragma once

template<typename Type>
struct mpArrayPtr
{
  Type* m_Data;
  size_t m_uiCount;

  mpArrayPtr() :
    m_Data(nullptr),
    m_uiCount(0)
  {
  }

  template<size_t N>
  mpArrayPtr(Type (&Data)[N]) :
    m_Data(Data),
    m_uiCount(N)
  {
  }

  mpArrayPtr(Type* Data, size_t uiCount) :
    m_Data(Data),
    m_uiCount(uiCount)
  {
  }

  ~mpArrayPtr()
  {
    m_Data = nullptr;
    m_uiCount = 0;
  }
};
