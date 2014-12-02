#pragma once

class Object
{
  sf::String m_sTextureName;
  sf::Texture m_Texture;
  sf::Sprite m_Sprite;

  friend sf::Sprite& SpriteOf(Object& object);
public:

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
};

MP_ForceInline sf::Sprite& SpriteOf(Object& object)
{
  return object.m_Sprite;
}
MP_ForceInline const sf::Sprite& SpriteOf(const Object& object)
{
  return SpriteOf(const_cast<Object&>(object));
}

MP_ForceInline auto Draw(const Object& object)
  -> decltype(MakeDrawInstruction(SpriteOf(object)))
{
  return MakeDrawInstruction(SpriteOf(object));
}
