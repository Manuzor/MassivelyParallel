#pragma once

namespace mpPlatform
{
  struct MP_WrapperAPI Handle
  {
    cl_platform_id m_PlatformId = nullptr;

    Handle(cl_platform_id platformId) : m_PlatformId(platformId) {}
  };

  MP_WrapperAPI Handle Get();
}
