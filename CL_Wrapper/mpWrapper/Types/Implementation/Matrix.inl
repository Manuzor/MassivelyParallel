
template<typename LhsType,
         typename RhsType,
         typename ResultType = mpMatrixTemplate<decltype(((typename LhsType::ElementType)1) * ((typename RhsType::ElementType)1))>>
MP_ForceInline
ResultType operator*(LhsType& Lhs,
                     RhsType& Rhs)
{
  MP_Assert(Lhs.GetWidth() == Rhs.GetHeight(), "The number of rows of the left matrix have to match the number of columns of the right matrix.");

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

    pScope->GetKernel().PushArg((cl_int)Lhs.GetHeight());
    pScope->GetKernel().PushArg((cl_int)Lhs.GetWidth());
    pScope->GetKernel().PushArg(LhsBuffer);
    pScope->GetKernel().PushArg((cl_int)Rhs.GetHeight());
    pScope->GetKernel().PushArg((cl_int)Rhs.GetWidth());
    pScope->GetKernel().PushArg(RhsBuffer);
    pScope->GetKernel().PushArg(ResultBuffer);

    {
      if (Result.GetElementCount() == 1)
      {
        pScope->GetKernel().Execute(1);
      }
      else
      {
        size_t Global[] = { Result.GetHeight(), Result.GetWidth() };
        pScope->GetKernel().Execute(Global);
      }
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
        ResultType::ElementType Current(0);

        for (size_t i = 0; i < Lhs.GetWidth(); ++i)
        {
          Current += Lhs(r, i) * Rhs(i, c);
        }

        Result(r, c) = Current;
      }
    }
  }

  return std::move(Result);
}
