#include "mpWrapper/PCH.h"
#include "mpWrapper/Utilities/Console.h"

#include <conio.h>

int mpUtilities::GetSingleCharacter()
{
  return _getch();
}
