#include "mpWrapper/PCH.h"
#include "mpWrapper/Utilities/File.h"

void mpLoadStringFromFile(std::vector<char>& out_Buffer, const char* szFileName)
{
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

  out_Buffer.reserve(1024);

  decltype(getc(nullptr)) Character;
  while ((Character = getc(pFile)) != EOF)
  {
    out_Buffer.push_back(Character);
  }
}
