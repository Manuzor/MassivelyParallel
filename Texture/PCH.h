#include "mpWrapper/PCH.h"
#include "mpWrapper/Core.h"
#include "mpWrapper/Utilities.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#define SFML_Verify(call) do{                           \
  if(!(call))                                           \
  {                                                     \
    mpLog::Error("Call failed: %s", #call);             \
    MP_ReportError("SFML call failed. Check the log."); \
  }                                                     \
} while(false)
