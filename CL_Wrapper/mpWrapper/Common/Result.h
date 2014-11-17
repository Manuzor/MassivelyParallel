#pragma once

struct mpResult
{
  MP_ForceInline static mpResult Success() { return CL_SUCCESS; }

  mpResult(cl_int iStatusCode) : m_Status(iStatusCode) {}

  MP_ForceInline bool Succeeded() { return m_Status == CL_SUCCESS; }
  MP_ForceInline bool Failed() { return m_Status != CL_SUCCESS; }

  cl_int m_Status;
};
