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

static bool g_bIsStarted = false;
static bool g_bIsShutDown = false;

void mpStartup::Startup()
{
  if(g_bIsShutDown)
  {
    // Reset to initial state.
    g_bIsStarted = false;
    g_bIsShutDown = false;
  }

  if(g_bIsStarted)
    return;

  for (auto pInit = g_pInit; pInit != nullptr; pInit = pInit->m_pNext)
  {
    pInit->OnStartup();
  }

  g_bIsStarted = true;
}

void mpStartup::Shutdown()
{
  if(g_bIsShutDown)
    return;

  MP_Assert(g_bIsStarted, "Cannot shut down when we are not started!");

  for (auto pInit = g_pInit; pInit != nullptr; pInit = pInit->m_pNext)
  {
    pInit->OnShutdown();
  }

  g_bIsShutDown = true;
}

mpStartup::mpInitialization::mpInitialization() :
  m_pNext(g_pInit)
{
  g_pInit = this;
}
