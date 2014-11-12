#pragma once

enum InitAsIdentity { Identity };
enum InitAsZero { Zero };
enum DoNotInit { NoInit };

MP_WrapperAPI void mpLoadMatrixFromFile(const char* szFileName, size_t uiWidth, size_t uiHeight, void* out_Data);

template<typename Type>
struct MatrixTemplate
{
  using ElementType = Type;

  MatrixTemplate(size_t uiHeight, size_t uiWidth) :
    m_uiHeight(uiHeight),
    m_uiWidth(uiWidth)
  {
    AllocateData();
  }

  MatrixTemplate(size_t uiHeight, size_t uiWidth, InitAsIdentity) :
    m_uiHeight(uiHeight),
    m_uiWidth(uiWidth)
  {
    MP_Assert(m_uiHeight == m_uiWidth, "Only quadratic matrices can be identity matrices");

    AllocateData();
    memset(m_Data, 0, GetByteCount());
    for(size_t i = 0; i < m_uiHeight; ++i)
    {
      At(i, i) = (ElementType)1;
    }
  }

  MatrixTemplate(size_t uiHeight, size_t uiWidth, InitAsZero) :
    m_uiHeight(uiHeight),
    m_uiWidth(uiWidth)
  {
    AllocateData();
    memset(m_Data, 0, Size);
  }

  MatrixTemplate(MatrixTemplate&& Other)  :
    m_uiHeight(Other.m_uiHeight),
    m_uiWidth(Other.m_uiWidth),
    m_Data(Other.m_Data)
  {
    Other.m_Data = nullptr;
    Other.m_uiWidth = 0;
    Other.m_uiHeight = 0;
  }

  MatrixTemplate(const MatrixTemplate& Other) :
    m_uiHeight(Other.m_uiHeight),
    m_uiWidth(Other.m_uiWidth)
  {
    AllocateData();
    memcpy(m_Data, Other.m_Data, Size);
  }

  void operator=(MatrixTemplate Copy)
  {
    using namespace std;
    swap(m_uiHeight, Copy.m_uiHeight);
    swap(m_uiWidth, Copy.m_uiWidth);
    swap(m_Data, Copy.m_Data);
  }

  void operator=(MatrixTemplate&& ToMove)
  {
    if(&ToMove == this)
      return;

    m_Data = ToMove.m_Data; ToMove.m_Data = nullptr;
    m_uiHeight = ToMove.m_uiHeight; ToMove.m_uiHeight = nullptr;
    m_uiWidth = ToMove.m_uiWidth; ToMove.m_uiWidth = nullptr;
  }

  ~MatrixTemplate()
  {
    ReleaseData();
  }

  //////////////////////////////////////////////////////////////////////////

  ElementType& At(size_t Y, size_t X)
  {
    MP_Assert(Y < m_uiHeight, "Row out of bounds");
    MP_Assert(X < m_uiWidth, "Col out of bounds");
    return m_Data[GetIndex(Y, X)];
  }

  const ElementType& At(size_t Y, size_t X) const
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
  MP_ForceInline static MatrixTemplate FromFile(const char* szFileName)
  {
    MatrixTemplate Result;
    mpLoadMatrixFromFile(szFileName, Result.Height, Result.Width, Result.m_Data);
    return std::move(Result);
  }

private:
  size_t m_uiWidth;
  size_t m_uiHeight;
  ElementType* m_Data;

  void AllocateData() { m_Data = new ElementType[GetElementCount()]; }
  void ReleaseData() { if(m_Data) { delete[] m_Data; m_Data = nullptr; } }

  MP_ForceInline size_t GetIndex(size_t Y, size_t X) const
  {
    return m_uiHeight * X + Y;
  }
};

#include "Wrapper/Types/Implementation/Matrix.inl"

using mpMatrix = MatrixTemplate<cl_float>;
