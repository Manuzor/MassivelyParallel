#pragma once

class MP_WrapperAPI mpPlatform
{
public:
  using Id = cl_platform_id;

  static mpPlatform Get();

public:
  Id m_Id = nullptr;

public:
  ~mpPlatform() { Shutdown(); }

private:
  mpPlatform() {}

  void Initialize();
  void Shutdown();
};
