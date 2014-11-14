#include "Wrapper/PCH.h"
#include "Wrapper/Startup.h"

mpApplication::mpApplication() :
  m_iArgumentCount(0),
  m_szArguments(nullptr),
  m_iExitCode(0)
{
}

void mpApplication::SetCommandLine(int argc, const char* argv[])
{
  m_iArgumentCount = argc;
  m_szArguments = argv;
}
