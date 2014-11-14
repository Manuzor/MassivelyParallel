#pragma once
#include "mpWrapper/Core/Device.h"

class MP_WrapperAPI mpContext
{
public:
  using Id = cl_context;

public:
  Id m_Id = nullptr;

public:
  mpContext(const mpContext&) = delete;
  void operator=(const mpContext&) = delete;

  mpContext() {}
  ~mpContext() { Release(); }

  void Initialize(const mpDevice& hDevice);
  void Release();
};
