
template<typename LhsType, size_t LhsRows, size_t LhsCols,
         typename RhsType, size_t RhsRows, size_t RhsCols,
         typename ResultType = decltype(((LhsType)1) * ((RhsType)1))>
MP_ForceInline
MatrixTemplate<ResultType, LhsRows, RhsCols> operator*(const MatrixTemplate<LhsType, LhsRows, LhsCols>& Lhs,
                                                       const MatrixTemplate<RhsType, RhsRows, RhsCols>& Rhs)
{
  static_assert(LhsRows == RhsCols, "The number of rows of the left matrix have to match the number of columns of the right matrix.");
  MatrixTemplate<ResultType, LhsRows, RhsCols> Result;

  return std::move(Result);
}
