#pragma once

class MP_WrapperAPI mpApplication
{
public:

  enum QuitOrContinue
  {
    Quit,
    Continue,
  };

public:

  virtual void PreStartup() {}
  virtual void PostStartup() {}

  virtual QuitOrContinue Run() = 0;

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

  /// \brief This class can be used for "global" startup and shutdown of any kind of system.
  class MP_WrapperAPI mpInitialization
  {
  public:
    mpInitialization();

    virtual void OnStartup() {}
    virtual void OnShutdown() {}

  private:
    mpInitialization* m_pNext;

    friend MP_WrapperAPI void Startup();
    friend MP_WrapperAPI void Shutdown();
  };
}

#define MP_GlobalInitializationBegin namespace { \
  class MP_Concat(_GlobalInitialization_, __LINE__) : public ::mpStartup::mpInitialization \
  { public:

#define MP_OnGlobalStartup virtual void OnStartup() final override
#define MP_OnGlobalShutdown virtual void OnShutdown() final override

#define MP_GlobalInitializationEnd } MP_Concat(_GlobalInitialization_, __LINE__) ## _Instance; }

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

    while(pApp->Run());

    pApp->PreShutdown();
    mpStartup::Shutdown();
    pApp->PostShutdown();

    auto iExitCode = pApp->GetExitCode();

    static_cast<AppType*>(pApp)->~AppType();
    return iExitCode;
  }
}

#include "mpWrapper/Foundation/Implementation/Startup.inl"

#define MP_EntryClass(AppType) int main(int argc, const char* argv[]) { return ::mpInternal::EntryPoint<AppType>(argc, argv); }
