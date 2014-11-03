#pragma once
#include "Wrapper/Context.h"
#include "Wrapper/Device.h"

class MP_WrapperAPI mpCommandQueue
{
public:
  using Id = cl_command_queue;

public:
  Id m_Id = nullptr;

public:
  mpCommandQueue() {}
  ~mpCommandQueue() { Release(); }

  void Initialize(const mpContext& Context, const mpDevice& Device);
  void Release();
private:
};
