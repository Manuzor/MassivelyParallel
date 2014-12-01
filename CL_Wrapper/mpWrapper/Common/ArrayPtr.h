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

  mpArrayPtr(nullptr_t) :
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

  MP_ForceInline Type& operator [](const size_t uiIndex)
  {
    MP_Assert(uiIndex < m_uiCount, "Index out of bounds");
    return m_Data[uiIndex];
  }
};

template<typename Type, size_t N>
mpArrayPtr<Type> mpMakeArrayPtr(Type (&Data)[N])
{
  return mpArrayPtr<Type>(Data);
}

template<typename Type>
mpArrayPtr<Type> mpMakeArrayPtr(Type* Data, size_t uiCount)
{
  return mpArrayPtr<Type>(Data, uiCount);
}
