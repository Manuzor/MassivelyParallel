#include "PCH.h"
#include "mpWrapper/Utilities/Console.h"
#include "mpWrapper/Types/Matrix.h"
#include "mpWrapper/Utilities.h"
#include "mpWrapper/Foundation/Startup.h"

#include "Drawing.h"
#include "Object.h"
#include "UserInput.h"

class Main : public mpApplication
{
  sf::RenderWindow  m_Window;
  Object m_Original;
  mpTime m_Beginning;
  UserInput m_UserInput;

  virtual void PreStartup() final override
  {
    m_Beginning = mpTime::Now();
  }

  virtual void PostStartup() final override
  {
    m_Window.create(sf::VideoMode(800, 600), "Texture Tiling");
    Initialize(m_Original, "Data/balls.png");
    Initialize(m_UserInput, "Data/Fonts/arial.ttf");
    TextOf(m_UserInput) += "Choose Texture: ";
  }

  void HandleText(sf::Event::TextEvent& text)
  {
    if(text.unicode < 32)
      return;

    Append(m_UserInput, text.unicode);
  }

  void HandleInput(sf::Event::KeyEvent& key)
  {
    switch(key.code)
    {
    case sf::Keyboard::Escape:
      m_Window.close();
      break;
    case sf::Keyboard::Return:
      Clear(m_UserInput);
      break;
    }
  }

  void HandleEvent(sf::Event& event)
  {
    switch(event.type)
    {
    case sf::Event::KeyReleased:
      HandleInput(event.key);
      break;
    case sf::Event::TextEntered:
      HandleText(event.text);
      break;
    case sf::Event::Closed:
      m_Window.close();
      break;
    }
  }

  virtual QuitOrContinue Run() final override
  {
    if (!m_Window.isOpen())
      return Quit;

    for(sf::Event event; m_Window.pollEvent(event);)
    {
      HandleEvent(event);
    }

    m_Window.clear();

    Draw(m_Window, m_Original);
    Draw(m_Window, m_UserInput);

    m_Window.display();

    return Continue;
  }

  virtual void PostShutdown() final override
  {
    mpLog::Info("The application ran for %f seconds.",
                (mpTime::Now() - m_Beginning).GetSeconds());
  }
};

MP_EntryClass(Main)
