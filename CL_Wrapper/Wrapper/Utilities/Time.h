/*
  Source code for mpTime is mostly taken from the ezEngine project
  (http://ezengine.net) and modified to suit the needs of this project.
*/

#pragma once

/// \brief The time class encapsulates a double value storing the time in seconds.
///
/// It offers convenient functions to get the time in other units.
/// mpTime is a high-precision time using the OS specific high-precision timing functions
/// and may thus be used for profiling as well as simulation code.
class MP_WrapperAPI mpTime
{
public:

  /// \brief Gets the current time
  static mpTime Now();

  /// \brief Creates an instance of mpTime that was initialized from nanoseconds.
  static mpTime Nanoseconds(double fNanoseconds)    { return mpTime(fNanoseconds * 0.000000001); }

  /// \brief Creates an instance of mpTime that was initialized from microseconds.
  static mpTime Microseconds(double fMicroseconds)  { return mpTime(fMicroseconds * 0.000001); }

  /// \brief Creates an instance of mpTime that was initialized from milliseconds.
  static mpTime Milliseconds(double fMilliseconds)  { return mpTime(fMilliseconds * 0.001); }

  /// \brief Creates an instance of mpTime that was initialized from seconds.
  static mpTime Seconds(double fSeconds)            { return mpTime(fSeconds); }

  mpTime() : m_fTime(0.0) {}

  /// \brief Sets the time value to zero.
  void SetZero();

  /// \brief Returns the time as a float value (in seconds).
  ///
  /// Useful for simulation time steps etc.
  /// Please note that it is not recommended to use the float value for long running
  /// time calculations since the precision can deteriorate quickly. (Only use for delta times is recommended)
  float AsFloat() const;

  /// \brief Returns the nanoseconds value
  double GetNanoseconds() const;

  /// \brief Returns the microseconds value
  double GetMicroseconds() const;

  /// \brief Returns the milliseconds value
  double GetMilliseconds() const;

  /// \brief Returns the seconds value.
  double GetSeconds() const;

  /// \brief Subtracts the time value of "other" from this instances value.
  void operator -= (const mpTime& other);

  /// \brief Adds the time value of "other" to this instances value.
  void operator += (const mpTime& other);

  /// \brief Returns the difference: "this instance - other"
  mpTime operator - (const mpTime& other) const;

  /// \brief Returns the sum: "this instance + other"
  mpTime operator + (const mpTime& other) const;

  bool operator< (const mpTime& rhs) const { return m_fTime <  rhs.m_fTime; }
  bool operator<=(const mpTime& rhs) const { return m_fTime <= rhs.m_fTime; }
  bool operator> (const mpTime& rhs) const { return m_fTime >  rhs.m_fTime; }
  bool operator>=(const mpTime& rhs) const { return m_fTime >= rhs.m_fTime; }
  bool operator==(const mpTime& rhs) const { return m_fTime == rhs.m_fTime; }
  bool operator!=(const mpTime& rhs) const { return m_fTime != rhs.m_fTime; }

private:

  /// \brief For internal use only.
  explicit mpTime(double fTime);

  /// \brief The time is stored in seconds
  double m_fTime;

private:
  static void Initialize();
};

mpTime operator* (mpTime t, double f);
mpTime operator* (double f, mpTime t);

mpTime operator/ (mpTime t, double f);
mpTime operator/ (double f, mpTime t);

#include "Wrapper/Utilities/Implementation/Time.inl"
