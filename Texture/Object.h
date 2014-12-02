#pragma once

class Object : public sf::Drawable
{
  // Data
  //////////////////////////////////////////////////////////////////////////
  sf::String m_sTextureName;
  sf::Texture m_Texture;
  sf::Sprite m_Sprite;

  // Friends
  //////////////////////////////////////////////////////////////////////////
  friend sf::Sprite& SpriteOf(Object& object);
  friend void Initialize(Object& object, const char* szTextureName);

  // sf::Drawable interface
  //////////////////////////////////////////////////////////////////////////
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const final override
  {
    target.draw(m_Sprite, states);
  }
};

MP_ForceInline sf::Sprite& SpriteOf(Object& object)
{
  return object.m_Sprite;
}
MP_ForceInline const sf::Sprite& SpriteOf(const Object& object)
{
  return SpriteOf(const_cast<Object&>(object));
}

MP_ForceInline
void Initialize(Object& object, const char* szTextureName)
{
  object.m_sTextureName = szTextureName;
  if(!object.m_Texture.loadFromFile(object.m_sTextureName))
  {
    mpLog::Error("Failed to load texture: %s", szTextureName);
    MP_ReportError("Failed to load texture.");
  }
  object.m_Texture.setSmooth(true);

  object.m_Sprite.setTexture(object.m_Texture);
}

MP_ForceInline
void Draw(sf::RenderTarget& target, const Object& object)
{
  target.draw(object);
}
