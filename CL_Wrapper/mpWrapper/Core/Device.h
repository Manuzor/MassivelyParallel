#pragma once
#include "mpWrapper/Core/Platform.h"

class MP_WrapperAPI mpDevice
{
public:
  using Id = cl_device_id;

  enum class Type
  {
    Invalid,

    CPU,
    GPU
  };

public:
  Type m_Type = Type::Invalid;
  Id m_Id;

  static mpDevice GetGPU(const mpPlatform& Platform, size_t uiIndex);
};
