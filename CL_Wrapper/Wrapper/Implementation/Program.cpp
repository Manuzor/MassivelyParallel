#include "Wrapper/PCH.h"
#include "Wrapper/Program.h"

static void LoadStringFromFile(std::stringstream& out_Buffer, const char* szFileName)
{
  // Windows specific
  bool bFileExists = GetFileAttributesA(szFileName) != INVALID_FILE_ATTRIBUTES;
  if (!bFileExists)
  {
    MP_ReportError("File not found.");
  }

  std::ifstream file;
  file.open(szFileName, std::fstream::in);
  if (!file.good())
  {
    MP_ReportError("Unable to open file.");
  }
  out_Buffer << file.rdbuf();
}

mpResult mpProgram::LoadAndBuild(const mpContext& Context,
                                 const mpDevice& Device,
                                 const char* szFileName)
{
  Release();

  std::stringstream Buffer;
  LoadStringFromFile(Buffer, szFileName);
  mpLog::Success("Loaded contents of program file '%s'", szFileName);

  auto Content = Buffer.str();
  auto ContentCString = Content.c_str();
  auto ContentCharCount = Content.size();

  cl_int status;
  m_Id = clCreateProgramWithSource(Context.m_Id, 1, &ContentCString, &ContentCharCount, &status);
  MP_Verify(status);
  mpResult buildResult = clBuildProgram(m_Id, 1, &Device.m_Id, nullptr, nullptr, nullptr);

  if(buildResult.Failed())
  {
    size_t bufferSize = 1024 * 1024;
    auto buffer = new char[bufferSize];
    MP_OnScopeExit{ delete[] buffer; };
    clGetProgramBuildInfo(m_Id, Device.m_Id, CL_PROGRAM_BUILD_LOG, bufferSize, buffer, NULL);
    mpLog::Error("========== Build Failed ==========");
    mpLog::Error(buffer);
    MP_ReportError("Build failed.");
  }
  else
  {
    mpLog::Success("%s: Build succeeded", szFileName);
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
