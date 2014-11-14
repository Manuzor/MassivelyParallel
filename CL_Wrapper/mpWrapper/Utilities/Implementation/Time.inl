
MP_ForceInline mpTime::mpTime(double fTime) : m_fTime(fTime)
{
}

MP_ForceInline void mpTime::SetZero()
{
  m_fTime = 0.0;
}

MP_ForceInline float mpTime::AsFloat() const
{
  return static_cast<float>(m_fTime);
}

MP_ForceInline double mpTime::GetNanoseconds() const
{
  return m_fTime * 1000000000.0;
}

MP_ForceInline double mpTime::GetMicroseconds() const
{
  return m_fTime * 1000000.0;
}

MP_ForceInline double mpTime::GetMilliseconds() const
{
  return m_fTime * 1000.0;
}

MP_ForceInline double mpTime::GetSeconds() const
{
  return m_fTime;
}

MP_ForceInline void mpTime::operator -= (const mpTime& other)
{
  m_fTime -= other.m_fTime;
}

MP_ForceInline void mpTime::operator += (const mpTime& other)
{
  m_fTime += other.m_fTime;
}

MP_ForceInline mpTime mpTime::operator - (const mpTime& other) const
{
  return mpTime(m_fTime - other.m_fTime);
}

MP_ForceInline mpTime mpTime::operator + (const mpTime& other) const
{
  return mpTime(m_fTime + other.m_fTime);
}

MP_ForceInline mpTime operator* (mpTime t, double f)
{
  return mpTime::Seconds(t.GetSeconds() * f);
}

MP_ForceInline mpTime operator* (double f, mpTime t)
{
  return mpTime::Seconds(t.GetSeconds() * f);
}

MP_ForceInline mpTime operator/ (mpTime t, double f)
{
  return mpTime::Seconds(t.GetSeconds() / f);
}

MP_ForceInline mpTime operator/ (double f, mpTime t)
{
  return mpTime::Seconds(t.GetSeconds() / f);
}

