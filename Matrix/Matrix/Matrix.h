#pragma once

enum InitAsIdentity { Identity };
enum InitAsZero { Zero };
enum DoNotInit { NoInit };

template<typename Type, size_t NumRows, size_t NumCols>
struct MatrixTemplate
{
  using ElementType = Type;

  enum
  {
    Rows = NumRows,
    Cols = NumCols,

    Height = Rows, // Alias
    Width = Cols, // Alias

    Count = NumRows * NumCols,
    Size = Count * sizeof(ElementType),
  };

  ElementType* m_Data;

  MatrixTemplate()
  {
    AllocateData();
  }

  MatrixTemplate(InitAsIdentity)
  {
    static_assert(Rows == Cols, "Only quadratic matrices can be identity matrices");

    AllocateData();
    memset(m_Data, 0, Size);
    for(size_t i = 0; i < MP_Min(Rows, Cols); ++i)
    {
      m_Data[i * i] = (ElementType)1;
    }
  }

  MatrixTemplate(InitAsZero)
  {
    AllocateData();
    memset(m_Data, 0, Size);
  }

  template<typename OtherType>
  MatrixTemplate(MatrixTemplate<OtherType, Rows, Cols>&& Other) :
    m_Data(Other.m_Data)
  {
    Other.m_Data = nullptr;
  }

  MatrixTemplate(const MatrixTemplate&) = delete;
  void operator=(const MatrixTemplate&) = delete;

  ~MatrixTemplate()
  {
    ReleaseData();
  }

  //////////////////////////////////////////////////////////////////////////

  template<size_t Row, size_t Col>
  ElementType& At()
  {
    static_assert(Row < Rows, "Row out of bounds");
    static_assert(Col < Cols, "Col out of bounds");
    return m_Data[GetIndex(Row, Col)];
  }

  template<size_t Row, size_t Col>
  const ElementType& At() const
  {
    static_assert(Row < Rows, "Row out of bounds");
    static_assert(Col < Cols, "Col out of bounds");
    return m_Data[GetIndex(Row, Col)];
  }

  ElementType& At(size_t Row, size_t Col)
  {
    MP_Assert(Row < Rows, "Row out of bounds");
    MP_Assert(Col < Cols, "Col out of bounds");
    return m_Data[GetIndex(Row, Col)];
  }

  const ElementType& At(size_t Row, size_t Col) const
  {
    MP_Assert(Row < Rows, "Row out of bounds");
    MP_Assert(Col < Cols, "Col out of bounds");
    return m_Data[GetIndex(Row, Col)];
  }

private:
  void AllocateData() { m_Data = new ElementType[Count]; }
  void ReleaseData() { if(m_Data) { delete[] m_Data; m_Data = nullptr; }}

  MP_ForceInline static size_t GetIndex(size_t Row, size_t Col)
  {
    return Height * Col + Row;
  }
};

#include "Matrix/Implementation/Matrix.inl"

template<size_t Rows, size_t Cols>
using Matrix = MatrixTemplate<cl_float, Rows, Cols>;

static_assert(Matrix<2, 4>::Cols == 4, "");
static_assert(Matrix<2, 4>::Rows == 2, "");
static_assert(Matrix<2, 4>::Count == 4 * 2, "");
static_assert(Matrix<2, 4>::Size == Matrix<2, 4>::Count * sizeof(cl_float), "");
