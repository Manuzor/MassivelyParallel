#pragma once

namespace mpInternal
{
  struct mpProfileScope
  {
    mpTime m_Begin;
    std::string m_Name;

    mpProfileScope(const char* szName) : m_Begin(mpTime::Now()), m_Name(szName)
    {
    }

    ~mpProfileScope()
    {
      mpLog::SeriousWarning("'%s' finished in %f seconds", m_Name.c_str(), (mpTime::Now() - m_Begin).GetSeconds());
    }
  };
}

#define MP_Profile(szName) ::mpInternal::mpProfileScope MP_Concat(_profile_, __LINE__)(szName)
#define MP_ProfiledLogBlock(szName) MP_LogBlock("Profiling: %s", szName); ::mpInternal::mpProfileScope MP_Concat(_profile_, __LINE__)(szName)
