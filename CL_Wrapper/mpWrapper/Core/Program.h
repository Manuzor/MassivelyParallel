#pragma once
#include "mpWrapper/Core/Context.h"
#include "mpWrapper/Core/Device.h"

class MP_WrapperAPI mpProgram
{
public:
  using Id = cl_program;

public:
  Id m_Id = nullptr;

public:
  mpProgram() {}
  ~mpProgram() { Release(); }

  mpResult LoadAndBuild(const mpContext& Context, const mpDevice& Device, const char* szFileName);
  void Release();
private:
};
