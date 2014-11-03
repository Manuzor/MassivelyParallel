#pragma once
#include "Wrapper/Program.h"

class MP_WrapperAPI mpKernel
{
public:
  using Id = cl_kernel;

  Id m_Id = nullptr;
public:

  ~mpKernel() { Release(); }

  void Initialize(const mpProgram& Program, const char* szKernelName);
  void Release();
};
