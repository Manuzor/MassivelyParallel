#include "PCH.h"
#include "Object.h"

static void PostInit(Object& object)
{
  TextureOf(object).setSmooth(true);
  TextureOf(object).setRepeated(true);
  SpriteOf(object).setTexture(TextureOf(object));
}

void Initialize(Object& object, mpUInt32 uiWidth, mpUInt32 uiHeight)
{
  object.m_sTextureName = "<Generated>";
  if(!object.m_Texture.create(uiWidth, uiHeight))
  {
    mpLog::Error("Failed to generate texture with dimensions %ux%u", uiWidth, uiHeight);
    MP_ReportError("Failed to generate texture.");
  }

  sf::Image img;
  img.create(uiWidth, uiHeight, sf::Color::White);
  object.m_Texture.update(img);

  PostInit(object);
}

void Initialize(Object& object, const char* szTextureName)
{
  object.m_sTextureName = szTextureName;
  if(!object.m_Texture.loadFromFile(object.m_sTextureName))
  {
    mpLog::Error("Failed to load texture: %s", szTextureName);
    MP_ReportError("Failed to load texture.");
  }

  PostInit(object);
}
