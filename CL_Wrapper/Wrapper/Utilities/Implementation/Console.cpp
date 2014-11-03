#include "Wrapper/PCH.h"
#include "Wrapper/Utilities/Console.h"

#include <conio.h>

int mpUtilities::GetSingleCharacter()
{
  return _getch();
}
