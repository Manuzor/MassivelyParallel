
template<typename LhsType,
         typename RhsType,
         typename ResultElementType = decltype(((typename LhsType::ElementType)1) * ((typename RhsType::ElementType)1)),
         typename ResultType = MatrixTemplate<ResultElementType>>
MP_ForceInline
ResultType operator*(LhsType& Lhs,
                     RhsType& Rhs)
{
  MP_Assert(Lhs.GetHeight() == Rhs.GetWidth(), "The number of rows of the left matrix have to match the number of columns of the right matrix.");

  ResultType Result(Lhs.GetHeight(), Rhs.GetWidth());

  auto pScope = mpGPUScope::GetCurrentScope();
  if (pScope)
  {
    // Use GPU
    //////////////////////////////////////////////////////////////////////////
    mpBuffer LhsBuffer;
    LhsBuffer.Initialize(pScope->GetContext(),
                         mpBufferFlags::ReadOnly,
                         mpMakeArrayPtr(Lhs.GetData(), Lhs.GetElementCount()));

    mpBuffer RhsBuffer;
    RhsBuffer.Initialize(pScope->GetContext(),
                         mpBufferFlags::ReadOnly,
                         mpMakeArrayPtr(Rhs.GetData(), Rhs.GetElementCount()));

    mpBuffer ResultBuffer;
    ResultBuffer.Initialize(pScope->GetContext(),
                            mpBufferFlags::WriteOnly,
                            Result.GetByteCount());

    pScope->GetKernel().PushArg(LhsBuffer);
    pScope->GetKernel().PushArg(RhsBuffer);
    pScope->GetKernel().PushArg(ResultBuffer);

    {
      size_t Global[] = { Result.GetHeight(), Result.GetWidth() };
      pScope->GetKernel().Execute(Global);
    }

    // Blocks until the kernel finished executing.
    ResultBuffer.ReadInto(mpMakeArrayPtr(Result.GetData(), Result.GetElementCount()), pScope->GetCommandQueue());
  }
  else
  {
    // Use CPU
    //////////////////////////////////////////////////////////////////////////
    for (size_t r = 0; r < Lhs.GetHeight(); ++r)
    {
      for (size_t c = 0; c < Rhs.GetWidth(); ++c)
      {
        ResultElementType Current(0);

        for (size_t i = 0; i < Lhs.GetWidth(); ++i)
        {
          Current += Lhs.At(r, i) * Rhs.At(i, c);
        }

        Result.At(r, c) = Current;
      }
    }
  }

  return std::move(Result);
}
