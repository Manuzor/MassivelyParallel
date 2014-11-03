#pragma once

template<typename Type, size_t Cols, size_t Rows>
struct Matrix
{
  Type m_Data[Cols * Rows];
  size_t m_DataCount;
  size_t m_DataSize;

  Matrix() :
    m_DataCount(Cols * Rows),
    m_DataSize(m_DataCount * sizeof(Type))
  {
    memset(m_Data, 0, m_DataSize);
  }
};
