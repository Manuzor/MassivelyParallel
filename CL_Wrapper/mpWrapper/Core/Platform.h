#pragma once

class MP_WrapperAPI mpPlatform
{
public:
  using Id = cl_platform_id;

public:
  Id m_Id = nullptr;

public:
  ~mpPlatform() { Shutdown(); }

  void Initialize();
  void Shutdown();
};
