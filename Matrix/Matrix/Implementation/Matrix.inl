
template<typename LhsType,
         typename RhsType,
         typename ResultElementType = decltype(((typename LhsType::ElementType)1) * ((typename RhsType::ElementType)1)),
         typename ResultType = MatrixTemplate<ResultElementType, LhsType::Rows, RhsType::Cols>>
MP_ForceInline
ResultType operator*(const LhsType& Lhs,
                     const RhsType& Rhs)
{
  static_assert(LhsType::Rows == RhsType::Cols, "The number of rows of the left matrix have to match the number of columns of the right matrix.");

  ResultType Result;

  auto pScope = mpGPUScope::GetCurrentScope();
  if (pScope)
  {
    // Use GPU
    //////////////////////////////////////////////////////////////////////////
    mpBuffer LhsBuffer;
    LhsBuffer.Initialize(pScope->GetContext(), mpBufferFlags::ReadOnly, mpMakeArrayPtr(Lhs.m_Data, Lhs.Count));

    mpBuffer RhsBuffer;
    RhsBuffer.Initialize(pScope->GetContext(), mpBufferFlags::ReadOnly, mpMakeArrayPtr(Rhs.m_Data, Rhs.Count));

    mpBuffer ResultBuffer;
    ResultBuffer.Initialize(pScope->GetContext(), mpBufferFlags::WriteOnly, Result.Size);

    pScope->GetKernel().PushArg(LhsBuffer);
    pScope->GetKernel().PushArg(RhsBuffer);
    pScope->GetKernel().PushArg(ResultBuffer);

    {
      size_t Global[] = { Result.Rows, Result.Cols };
      pScope->GetKernel().Execute(Global);
    }

    // Blocks until the kernel finished executing.
    ResultBuffer.ReadInto(mpMakeArrayPtr(Result.m_Data, Result.Count), pScope->GetCommandQueue());
  }
  else
  {
    // Use CPU
    //////////////////////////////////////////////////////////////////////////
    for (size_t r = 0; r < LhsType::Rows; ++r)
    {
      for (size_t c = 0; c < RhsType::Cols; ++c)
      {
        ResultElementType Current(0);

        for (size_t i = 0; i < LhsType::Cols; ++i)
        {
          Current += Lhs.At(r, i) * Rhs.At(i, c);
        }

        Result.At(r, c) = Current;
      }
    }
  }

  return std::move(Result);
}
