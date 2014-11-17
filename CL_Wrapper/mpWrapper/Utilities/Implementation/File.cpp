#include "mpWrapper/PCH.h"
#include "mpWrapper/Utilities/File.h"
#include "mpWrapper/Utilities/Time.h"

void mpLoadStringFromFile(std::vector<char>& out_Buffer, const char* szFileName)
{
  MP_LogBlock("mpLoadStringFromFile (%s)", szFileName);
  auto Beginning = mpTime::Now();

  bool bFileExists = GetFileAttributesA(szFileName) != INVALID_FILE_ATTRIBUTES;
  if (!bFileExists)
  {
    MP_ReportError("File not found.");
  }

  FILE* pFile = fopen(szFileName, "r");
  if (pFile == nullptr)
  {
    MP_ReportError("Unable to open file.");
  }
  MP_OnScopeExit{ fclose(pFile); };
  auto OpenedFile = mpTime::Now();
  mpLog::Dev("Opened file in %f seconds.", (OpenedFile - Beginning).GetSeconds());

  out_Buffer.reserve(1024);

  decltype(getc(nullptr)) Character;
  while ((Character = getc(pFile)) != EOF)
  {
    out_Buffer.push_back(Character);
  }

  auto End = mpTime::Now();
  mpLog::Dev("Read all characters in %f seconds", (End - OpenedFile).GetSeconds());
  mpLog::Dev("Finished in %f seconds", (End - Beginning).GetSeconds());
}
