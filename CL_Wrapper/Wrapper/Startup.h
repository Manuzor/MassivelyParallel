#pragma once

class MP_WrapperAPI mpApplication
{
public:

  enum class RunBehavior
  {
    Quit,
    Continue,
  };

public:

  virtual void PreStartup() {}
  virtual void PostStartup() {}

  virtual RunBehavior Run() = 0;

  virtual void PreShutdown() {}
  virtual void PostShutdown() {}

public:
  mpApplication();

  void SetCommandLine(int argc, const char* argv[]);

  void SetExitCode(int iExitCode) { m_iExitCode = iExitCode; }
  int GetExitCode() const { return m_iExitCode; }

private:
  int m_iArgumentCount;
  const char** m_szArguments;
  int m_iExitCode;
};

namespace mpStartup
{
  MP_WrapperAPI void Startup();
  MP_WrapperAPI void Shutdown();
}

namespace mpInternal
{
  template<typename AppType>
  int EntryPoint(int argc, const char* argv[])
  {
    static char AppMemory[sizeof(AppType)];
    auto pApp = static_cast<mpApplication*>(new (AppMemory) AppType);

    pApp->SetCommandLine(argc, argv);

    pApp->PreStartup();
    mpStartup::Startup();
    pApp->PostStartup();

    while(pApp->Run() == mpApplication::RunBehavior::Continue);

    pApp->PreShutdown();
    mpStartup::Shutdown();
    pApp->PostShutdown();

    auto iExitCode = pApp->GetExitCode();

    static_cast<AppType*>(pApp)->~AppType();
    return iExitCode;
  }
}

#include "Wrapper/Implementation/Startup.inl"

#define MP_EntryClass(AppType) int main(int argc, const char* argv[]) { return ::mpInternal::EntryPoint<AppType>(argc, argv); }
