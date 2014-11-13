
typedef struct
{
  int m_uiHeight;
  int m_uiWidth;
  global float* m_Data;
} Matrix;

int GetHeight(Matrix M)
{
  return M.m_uiHeight;
}

int GetWidth(Matrix M)
{
  return M.m_uiWidth;
}

global float* GetData(Matrix M)
{
  return M.m_Data;
}

int GetIndex(Matrix M, int Y, int X)
{
  return GetHeight(M) * X + Y;
}

void SetAt(Matrix M, int Y, int X, float fValue)
{
  GetData(M)[GetIndex(M, Y, X)] = fValue;
}

float GetAt(Matrix M, int Y, int X)
{
  return GetData(M)[GetIndex(M, Y, X)];
}

void Multiply(Matrix A, Matrix B, Matrix R)
{
  // width of A must be equal to height of B

  float fCurrent = 0;
  for(int i = 0; i < GetWidth(A); ++i)
  {
    fCurrent += GetAt(A, get_global_id(0), i) * GetAt(B, i, get_global_id(1));
  }
  SetAt(R, get_global_id(0), get_global_id(1), fCurrent);
}

kernel void MultiplyMatrix(int HeightA, int WidthA, global float* DataA,
                           int HeightB, int WidthB, global float* DataB,
                                                    global float* Result)
{
  Matrix A = { HeightA, WidthA, DataA  };
  Matrix B = { HeightB, WidthB, DataB  };
  Matrix R = { HeightA, WidthB, Result };

  Multiply(A, B, R);
}
