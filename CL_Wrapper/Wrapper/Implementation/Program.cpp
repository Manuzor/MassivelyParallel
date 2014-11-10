#include "Wrapper/PCH.h"
#include "Wrapper/Program.h"
#include "Wrapper/Utilities/File.h"

mpResult mpProgram::LoadAndBuild(const mpContext& Context,
                                 const mpDevice& Device,
                                 const char* szFileName)
{
  Release();

  std::vector<char> Content;
  mpLoadStringFromFile(Content, szFileName);

  const char* ContentCString = Content.data();
  auto ContentCharCount = Content.size();

  cl_int status;
  m_Id = clCreateProgramWithSource(Context.m_Id, 1, &ContentCString, &ContentCharCount, &status);
  MP_Verify(status);

  mpLog::Info("Building '%s' . . .", szFileName);
  mpResult buildResult = clBuildProgram(m_Id, 1, &Device.m_Id, nullptr, nullptr, nullptr);

  if(buildResult.Failed())
  {
    size_t bufferSize = 1024 * 1024;
    auto buffer = new char[bufferSize];
    MP_OnScopeExit{ delete[] buffer; };
    MP_Verify(clGetProgramBuildInfo(m_Id, Device.m_Id, CL_PROGRAM_BUILD_LOG, bufferSize, buffer, NULL));
    mpLog::Error("---------- Build Failed ----------\n%s", buffer);
  }

  return buildResult;
}

void mpProgram::Release()
{
  if(m_Id == nullptr)
    return;

  MP_Verify(clReleaseProgram(m_Id));
  m_Id = nullptr;
}
