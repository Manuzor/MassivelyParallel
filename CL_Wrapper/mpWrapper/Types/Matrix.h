#pragma once

enum InitAsIdentity { Identity };
enum InitAsZero { Zero };
enum DoNotInit { NoInit };

namespace mpInternal
{
  MP_WrapperAPI void mpLoadMatrixFromFile(std::vector<cl_float>& out_Data, size_t& out_uiHeight, size_t& out_uiWidth, const char* szFileName);
}

template<typename Type>
struct mpMatrixTemplate
{
  using ElementType = Type;

  mpMatrixTemplate(DoNotInit) : m_uiHeight(0), m_uiWidth(0), m_Data(nullptr) {}

  mpMatrixTemplate(size_t uiHeight, size_t uiWidth) :
    m_uiHeight(uiHeight),
    m_uiWidth(uiWidth)
  {
    AllocateData();
  }

  mpMatrixTemplate(size_t uiHeight, size_t uiWidth, InitAsIdentity) :
    m_uiHeight(uiHeight),
    m_uiWidth(uiWidth)
  {
    MP_Assert(m_uiHeight == m_uiWidth, "Only quadratic matrices can be identity matrices");

    AllocateData();
    memset(m_Data, 0, GetByteCount());
    for(size_t i = 0; i < m_uiHeight; ++i)
    {
      Deref(this)(i, i) = (ElementType)1;
    }
  }

  mpMatrixTemplate(size_t uiHeight, size_t uiWidth, InitAsZero) :
    m_uiHeight(uiHeight),
    m_uiWidth(uiWidth)
  {
    AllocateData();
    memset(m_Data, 0, Size);
  }

  mpMatrixTemplate(mpMatrixTemplate&& Other)  :
    m_uiHeight(Other.m_uiHeight),
    m_uiWidth(Other.m_uiWidth),
    m_Data(Other.m_Data)
  {
    Other.m_Data = nullptr;
    Other.m_uiWidth = 0;
    Other.m_uiHeight = 0;
  }

  mpMatrixTemplate(const mpMatrixTemplate& Other) :
    m_uiHeight(Other.m_uiHeight),
    m_uiWidth(Other.m_uiWidth)
  {
    AllocateData();
    memcpy(m_Data, Other.m_Data, GetByteCount());
  }

  void operator=(const mpMatrixTemplate& ToCopy)
  {
    mpMatrixTemplate Copy(ToCopy);

    using namespace std;
    swap(m_uiHeight, Copy.m_uiHeight);
    swap(m_uiWidth, Copy.m_uiWidth);
    swap(m_Data, Copy.m_Data);
  }

  void operator=(mpMatrixTemplate&& ToMove)
  {
    if(&ToMove == this)
      return;

    m_Data = ToMove.m_Data; ToMove.m_Data = nullptr;
    m_uiHeight = ToMove.m_uiHeight; ToMove.m_uiHeight = 0;
    m_uiWidth = ToMove.m_uiWidth; ToMove.m_uiWidth = 0;
  }

  ~mpMatrixTemplate()
  {
    ReleaseData();
  }

  //////////////////////////////////////////////////////////////////////////

  //ElementType& At(size_t Y, size_t X)
  ElementType& operator()(size_t Y, size_t X)
  {
    MP_Assert(Y < m_uiHeight, "Row out of bounds");
    MP_Assert(X < m_uiWidth, "Col out of bounds");
    return m_Data[GetIndex(Y, X)];
  }

  // const ElementType& At(size_t Y, size_t X) const
  const ElementType& operator()(size_t Y, size_t X) const
  {
    MP_Assert(Y < m_uiHeight, "Row out of bounds");
    MP_Assert(X < m_uiWidth, "Col out of bounds");
    return m_Data[GetIndex(Y, X)];
  }

  MP_ForceInline size_t GetWidth() const { return m_uiWidth; }
  MP_ForceInline size_t GetHeight() const { return m_uiHeight; }
  MP_ForceInline ElementType* GetData() { return m_Data; }
  MP_ForceInline const ElementType* GetData() const { return m_Data; }

  MP_ForceInline size_t GetElementCount() const { return m_uiWidth * m_uiHeight; }
  MP_ForceInline size_t GetByteCount() const { return GetElementCount() * sizeof(ElementType); }

  // Static
  //////////////////////////////////////////////////////////////////////////
  MP_ForceInline static mpMatrixTemplate FromFile(const char* szFileName)
  {
    std::vector<ElementType> Data;
    size_t uiHeight, uiWidth;
    mpInternal::mpLoadMatrixFromFile(Data, uiHeight, uiWidth, szFileName);
    MP_Assert(Data.size() == uiWidth * uiHeight, "Invalid output");
    auto Result = mpMatrixTemplate(uiHeight, uiWidth);
    memcpy(Result.m_Data, Data.data(), Result.GetByteCount());
    return std::move(Result);
  }

private:
  size_t m_uiWidth;
  size_t m_uiHeight;
  ElementType* m_Data;

  MP_ForceInline void AllocateData()
  {
    if(GetElementCount() > 0)
      m_Data = new ElementType[GetElementCount()];
    else
      m_Data = nullptr;
  }

  MP_ForceInline void ReleaseData()
  {
    if(m_Data)
    {
      delete[] m_Data;
      m_Data = nullptr;
    }
  }

  MP_ForceInline size_t GetIndex(size_t Y, size_t X) const
  {
    return m_uiHeight * X + Y;
  }
};

#include "mpWrapper/Types/Implementation/Matrix.inl"

using mpMatrix = mpMatrixTemplate<cl_float>;
