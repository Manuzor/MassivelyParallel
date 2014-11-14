#include "mpWrapper/PCH.h"
#include "mpWrapper/Core/Device.h"

namespace
{
  struct DeviceCache
  {
    const mpDevice::Type m_Type;
    cl_uint m_uiNumDevices;
    const cl_uint m_uiMaxNumDevices = 32;
    enum { MaxNumDevices = 32 };
    mpDevice::Id m_Devices[MaxNumDevices];

    DeviceCache(mpDevice::Type DeviceType) : m_Type(DeviceType) {}
  };
}

static DeviceCache g_GPUCache(mpDevice::Type::GPU);
static DeviceCache g_CPUCache(mpDevice::Type::CPU);

mpDevice mpDevice::GetGPU(const mpPlatform& Platform, size_t uiIndex)
{
  mpDevice result;
  result.m_Type = Type::GPU;

  cl_uint numDevices = 0;
  MP_Verify(clGetDeviceIDs(Platform.m_Id, CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices));
  MP_Assert(uiIndex < numDevices, "Index out of range");
  if (numDevices == 0) // no GPU available.
  {
    MP_ReportError("No GPU device available.");
  }
  else
  {
    if (numDevices > DeviceCache::MaxNumDevices)
    {
      MP_ReportError("Maximum number of supported devices reached!");
    }
    MP_Verify(clGetDeviceIDs(Platform.m_Id, CL_DEVICE_TYPE_GPU, numDevices, g_GPUCache.m_Devices, NULL));
    result.m_Id = g_GPUCache.m_Devices[uiIndex];
  }

  return result;
}
