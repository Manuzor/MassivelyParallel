#include "mpWrapper/PCH.h"
#include "mpWrapper/Core/Program.h"
#include "mpWrapper/Utilities/File.h"
#include "mpWrapper/Utilities/Time.h"

mpResult mpProgram::LoadAndBuild(const mpContext& Context,
                                 const mpDevice& Device,
                                 const char* szFileName)
{
  MP_LogBlock("Program Load and Build (%s)", szFileName);

  Release();

  {
    MP_LogBlock("Load");

    std::vector<char> Content;
    mpLoadStringFromFile(Content, szFileName);

    const char* ContentCString = Content.data();
    auto ContentCharCount = Content.size();

    cl_int status;
    m_Id = clCreateProgramWithSource(Context.m_Id, 1, &ContentCString, &ContentCharCount, &status);
    MP_Verify(status);
  }

  {
    MP_LogBlock("Build");

    mpLog::Info("Building . . .");
    auto Beginning = mpTime::Now();
    mpResult buildResult = clBuildProgram(m_Id, 1, &Device.m_Id, nullptr, nullptr, nullptr);

    if(buildResult.Failed())
    {
      size_t bufferSize = 1024 * 1024;
      auto buffer = new char[bufferSize];
      MP_OnScopeExit{ delete[] buffer; };
      MP_Verify(clGetProgramBuildInfo(m_Id, Device.m_Id, CL_PROGRAM_BUILD_LOG, bufferSize, buffer, NULL));
      mpLog::Error("---------- Build Failed ----------\n%s%s", szFileName, buffer);
      return buildResult;
    }

    mpLog::Success("Built program in %f seconds", mpTime::Now() - Beginning);
  }

  return mpResult::Success();
}

void mpProgram::Release()
{
  if(m_Id == nullptr)
    return;

  MP_Verify(clReleaseProgram(m_Id));
  m_Id = nullptr;
}
