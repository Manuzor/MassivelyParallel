#include "PCH.h"
#include "mpWrapper/Utilities/Console.h"
#include "mpWrapper/Types/Matrix.h"
#include "mpWrapper/Utilities.h"
#include "mpWrapper/Foundation/Startup.h"

class Object
{
public:
  sf::String m_sTextureName;
  sf::Texture m_Texture;
  sf::Sprite m_Sprite;

  void Initialize(const char* szTextureName)
  {
    m_sTextureName = szTextureName;
    if(!m_Texture.loadFromFile(m_sTextureName))
    {
      mpLog::Error("Failed to load texture: %s", szTextureName);
      MP_ReportError("Failed to load texture.");
    }
    m_Texture.setSmooth(true);

    m_Sprite.setTexture(m_Texture);
  }

  void Draw(sf::RenderWindow& Window)
  {
    Window.draw(m_Sprite);
  }
};

class Main : public mpApplication
{
  sf::RenderWindow  m_Window;
  Object m_Original;
  mpTime m_Beginning;

  virtual void PreStartup() final override
  {
    m_Beginning = mpTime::Now();
  }

  virtual void PostStartup() final override
  {
    m_Window.create(sf::VideoMode(800, 600), "Texture Tiling");
    m_Original.Initialize("Data/balls.png");
  }

  void HandleEvent(sf::Event& event)
  {
    switch(event.type)
    {
    case sf::Event::KeyReleased:
      if (event.key.code != sf::Keyboard::Escape)
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

    sf::Event event;
    while(m_Window.pollEvent(event))
    {
      HandleEvent(event);
    }

    m_Window.clear();

    m_Original.Draw(m_Window);

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
