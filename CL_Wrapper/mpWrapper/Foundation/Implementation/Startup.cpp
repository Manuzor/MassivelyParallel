#include "mpWrapper/PCH.h"
#include "mpWrapper/Foundation/Startup.h"

namespace
{
  mpStartup::mpInitialization* g_pInit{ nullptr };

}

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

void mpStartup::Startup()
{
  for (auto pInit = g_pInit; pInit != nullptr; pInit = pInit->m_pNext)
  {
    pInit->OnStartup();
  }
}

void mpStartup::Shutdown()
{
  for (auto pInit = g_pInit; pInit != nullptr; pInit = pInit->m_pNext)
  {
    pInit->OnShutdown();
  }
}

mpStartup::mpInitialization::mpInitialization() :
  m_pNext(g_pInit)
{
  g_pInit = this;
}
