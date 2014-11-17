
inline const char* mpLogLevel::GetString()
{
  static_assert(COUNT == 8, "Update this function.");
  switch(m_Value)
  {
  case mpLogLevel::BlockBegin:     return "BlockBegin";
  case mpLogLevel::BlockEnd:       return "BlockEnd";
  case mpLogLevel::None:           return "None";
  case mpLogLevel::Error:          return "Error";
  case mpLogLevel::SeriousWarning: return "SeriousWarning";
  case mpLogLevel::Warning:        return "Warning";
  case mpLogLevel::Success:        return "Success";
  case mpLogLevel::Info:           return "Info";
  case mpLogLevel::Dev:            return "Dev";
  case mpLogLevel::Debug:          return "Debug";
  }
  MP_NotImplemented;
  return nullptr;
}

inline const char* mpLogLevel::GetShortString()
{
  static_assert(COUNT == 8, "Update this function.");
  switch(m_Value)
  {
  case mpLogLevel::BlockBegin:     return ">>>";
  case mpLogLevel::BlockEnd:       return "<<<";
  case mpLogLevel::None:           return "Non";
  case mpLogLevel::Error:          return "Err";
  case mpLogLevel::SeriousWarning: return "Srs";
  case mpLogLevel::Warning:        return "Wen";
  case mpLogLevel::Success:        return "Suc";
  case mpLogLevel::Info:           return "Ifo";
  case mpLogLevel::Dev:            return "Dev";
  case mpLogLevel::Debug:          return "Dbg";
  }
  MP_NotImplemented;
  return nullptr;
}
