#include "PCH.h"
#include "mpWrapper/Utilities/Console.h"
#include "mpWrapper/Types/Matrix.h"
#include "mpWrapper/Utilities.h"
#include "mpWrapper/Foundation/Startup.h"

class Main : public mpApplication
{
  sf::RenderWindow  m_Window;
  mpTime m_Beginning;

  virtual void PreStartup() final override
  {
    m_Beginning = mpTime::Now();
  }

  virtual void PostStartup() final override
  {
    m_Window.create(sf::VideoMode(800, 600), "Texture Tiling");
  }

  virtual QuitOrContinue Run() final override
  {
    if (!m_Window.isOpen())
      return Quit;

    sf::Event event;
    while(m_Window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      {
        m_Window.close();
      }
    }

    m_Window.clear();
    m_Window.display();

    return Continue;
  }

  virtual void PostShutdown() final override
  {
    mpLog::Info("The application ran for %f seconds.",
                (mpTime::Now() - m_Beginning).GetSeconds());
    printf("\nPress any key to quit . . . ");
    mpUtilities::GetSingleCharacter();
  }
};

MP_EntryClass(Main)
