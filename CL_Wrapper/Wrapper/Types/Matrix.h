#pragma once

enum InitAsIdentity { Identity };
enum InitAsZero { Zero };
enum DoNotInit { NoInit };

MP_WrapperAPI void mpLoadMatrixFromFile(const char* szFileName, size_t uiWidth, size_t uiHeight, void* out_Data);

template<typename Type, size_t NumRows, size_t NumCols>
struct MatrixTemplate
{
  using SelfType = MatrixTemplate<Type, NumRows, NumCols>;
  using ElementType = Type;

  enum
  {
    Height = NumRows, // Alias
    Width = NumCols, // Alias

    Count = Width * Height,
    Size = Count * sizeof(ElementType),
  };

  ElementType* m_Data;

  MatrixTemplate()
  {
    AllocateData();
  }

  MatrixTemplate(InitAsIdentity)
  {
    static_assert(Height == Width, "Only quadratic matrices can be identity matrices");

    AllocateData();
    memset(m_Data, 0, Size);
    for(size_t i = 0; i < MP_Min(Height, Width); ++i)
    {
      At(i, i) = (ElementType)1;
    }
  }

  MatrixTemplate(InitAsZero)
  {
    AllocateData();
    memset(m_Data, 0, Size);
  }

  template<typename OtherType>
  MatrixTemplate(MatrixTemplate<OtherType, Height, Width>&& Other) :
    m_Data(Other.m_Data)
  {
    Other.m_Data = nullptr;
  }

  MatrixTemplate(const MatrixTemplate& Other)
  {
    AllocateData();
    memcpy(m_Data, Other.m_Data, Size);
  }

  void operator=(const MatrixTemplate&) = delete;

  ~MatrixTemplate()
  {
    ReleaseData();
  }

  //////////////////////////////////////////////////////////////////////////

  template<size_t Y, size_t X>
  ElementType& At()
  {
    static_assert(Y < Height, "Row out of bounds");
    static_assert(X < Width, "Col out of bounds");
    return m_Data[GetIndex(Y, X)];
  }

  template<size_t Y, size_t X>
  const ElementType& At() const
  {
    static_assert(Y < Height, "Row out of bounds");
    static_assert(X < Width, "Col out of bounds");
    return m_Data[GetIndex(Y, X)];
  }

  ElementType& At(size_t Y, size_t X)
  {
    MP_Assert(Y < Height, "Row out of bounds");
    MP_Assert(X < Width, "Col out of bounds");
    return m_Data[GetIndex(Y, X)];
  }

  const ElementType& At(size_t Y, size_t X) const
  {
    MP_Assert(Y < Height, "Row out of bounds");
    MP_Assert(X < Width, "Col out of bounds");
    return m_Data[GetIndex(Y, X)];
  }

  // Static
  //////////////////////////////////////////////////////////////////////////
  MP_ForceInline static MatrixTemplate FromFile(const char* szFileName)
  {
    MatrixTemplate Result;
    mpLoadMatrixFromFile(szFileName, Result.Height, Result.Width, Result.m_Data);
    return std::move(Result);
  }

private:
  void AllocateData() { m_Data = new ElementType[Count]; }
  void ReleaseData() { if(m_Data) { delete[] m_Data; m_Data = nullptr; }}

  MP_ForceInline static size_t GetIndex(size_t Y, size_t X)
  {
    return Height * X + Y;
  }
};

#include "Wrapper/Types/Implementation/Matrix.inl"

template<size_t Rows, size_t Cols>
using mpMatrix = MatrixTemplate<cl_float, Rows, Cols>;

static_assert(mpMatrix<2, 4>::Height == 2, "");
static_assert(mpMatrix<2, 4>::Width == 4, "");
static_assert(mpMatrix<2, 4>::Count == 2 * 4, "");
static_assert(mpMatrix<2, 4>::Size == 2 * 4 * sizeof(cl_float), "");
